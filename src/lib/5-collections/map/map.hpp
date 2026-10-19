/* @DOC Performance of ca::map<K, V, A>.

   Layout: root pointer and size, 16 bytes on 64-bit, plus the allocator (nothing for ca::heap).
   A B-tree of order 12: each node holds up to 11 sorted keys in one array and their values in
   another, so a node search scans contiguous keys only; branches add 12 child pointers. Leaves
   are 16 bytes of header plus 12 keys and 12 values (one spare of each for the overflow before
   a split). An empty map owns no memory.

   | Operation | Time | Notes |
   |---|---|---|
   | get, contains, operator[] | O(log n) | log12(n) to log6(n) nodes, a linear scan of up to 11 keys in each |
   | set, setdefault | O(log n) | at most one node per level allocated, all before anything changes |
   | pop, popitem, pop_first | O(log n) | rotations and merges, no allocation |
   | first, last | O(log n) | one descent |
   | floor, ceiling | O(log n) | |
   | iteration, keys, values, items | O(n) | in key order; amortized O(1) per step |
   | retain | O(n + r log n) | r entries removed |
   | clear, cleanup, clone | O(n) | clone copies the tree shape, memcpy per node for trivially copyable entries |

   Space: nodes are at least half full (5 of 11 keys) except the root; for small entries the
   overhead per entry is about 16 / 8 + 8 / 6 bytes: far below a red-black tree's three pointers
   and color per entry.

   Locality: a lookup touches about log12(n) nodes, each scanned linearly in one or two cache
   lines of keys; a std::map node walk is a dependent load per level of log2(n).

   Relocation: entries move inside and between nodes on insertion and removal (memmove for
   trivially relocatable keys and values); positions and references do not survive them.
   TRIVIALLY_RELOCATABLE when A is: nodes point at their parent, never at the map.

   Copy and move: the copy constructor is deleted; clone() is the explicit, fallible copy. Move
   takes two words and the allocator.

   Baseline std::map over glibc malloc; ca over ca::heap; u64 keys and values unless said:
   --BENCH bench/map.cpp
   bench/map.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | insert 1e6 scrambled u64, then cleanup | 0.74x | 0.78x | entries shift within nodes |
   | insert 1e6 u64 in order | 0.24x | 0.32x | |
   | 3e6 lookup hits in 1e6 entries | 0.57x | 0.32x | |
   | 1e6 ceiling misses | 0.52x | 0.67x | std: lower_bound |
   | walk 1e6 entries 10 times | 0.14x | 0.15x | |
   | insert 1e6, then pop all in scrambled order | 0.31x | 0.31x | |
   | str keys, 1e5 inserted, 5e5 lookups by literal | 0.45x | 0.49x | std: std::less<> and string_view lookups |
   BENCH-- */

#ifndef ca_collections_map_map_hpp
#define ca_collections_map_map_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/memory/destroy_n.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/memory/relocate_n.hpp"
#include "1-primitives/memory/relocate_overlapping_n.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_move_constructible.hpp"
#include "1-primitives/traits/is_object.hpp"
#include "1-primitives/traits/is_trivially_copyable.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "1-primitives/utility/forward.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/in_place.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/is_cleanable.hpp"
#include "4-lifecycle/is_clonable.hpp"
#include "4-lifecycle/memory/allocate_n.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/deallocate_n.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "5-collections/map/item.hpp"
#include "5-collections/map/impl/btree_node.hpp"
#include "5-collections/map/impl/key.hpp"
#include "5-collections/map/impl/range.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MAP ----------------------------------------------------------------

	/* @DOC Map from K to V sorted by key, a B-tree in memory from A, with Python's dict API:
	   map[key] (traps when absent), get, set (d[key] = value), setdefault, pop, popitem (the
	   largest key), update, keys, values, items, from_keys, contains (in), size (len), clear;
	   clone is dict.copy. Ordered extras: first, last, pop_first, floor, ceiling, and iteration
	   in key order. Keys are looked up by lookup_type (an sv for str keys) and ordered by
	   compare() when they have it, else by <; set and setdefault build the key from what they
	   are given only when it is absent. Construction allocates nothing; set, setdefault, update
	   and clone acquire memory and can fail, changing nothing. Insertions and removals move
	   entries: positions and references do not survive them. Clean up with cleanup(); the
	   destructor is a failsafe that frees silently. A stateful A has no default construction
	   and moves with the entries. */
	template <typename K, typename V, ca::is_allocator A = ca::heap>
	class map final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::map<K, V, A>;

			using entry = ca::item<K, V>;

			using node = ca::impl::btree_node<K, V>;

			using branch = ca::impl::btree_branch<K, V>;

			struct position final {

					using self = typename ca::map<K, V, A>::position;

					node*   _node;
					ca::usz _index;
					bool    _found;

			}; // struct position


			// -- private constants ------------------------------------------

			static constexpr ca::usz _capacity = node::CAPACITY;

			static constexpr ca::usz _minimum = node::MINIMUM;

			static constexpr ca::usz _max_height = 48U;


		public:

			// -- public types -----------------------------------------------

			using key_type             = K;
			using mapped_type          = V;
			using value_type           = ca::item<K, V>;
			using lookup_type          = ca::impl::lookup_of<K>;
			using allocator_type       = A;
			using size_type            = ca::usz;
			using iterator             = ca::impl::btree_iterator<K, V, ca::impl::project_btree_item>;
			using const_iterator       = ca::impl::btree_iterator<K, const V, ca::impl::project_btree_item>;
			using key_iterator         = ca::impl::btree_iterator<K, const V, ca::impl::project_btree_key>;
			using value_iterator       = ca::impl::btree_iterator<K, V, ca::impl::project_btree_value>;
			using const_value_iterator = ca::impl::btree_iterator<K, const V, ca::impl::project_btree_value>;

			/* @DOC The same map type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::map<K, V, B>;


			// -- public constants -------------------------------------------

			/* @DOC The root pointer, the size and the allocator; no node points at the map:
			   containers move a map with memcpy when A allows it. */
			static constexpr bool TRIVIALLY_RELOCATABLE = ca::is_trivially_relocatable<A>;


		private:

			// -- private members --------------------------------------------

			node*                   _root;
			ca::usz                 _size;
			[[no_unique_address]] A _allocator;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr map(void) noexcept requires ca::is_stateless_allocator<A>
			: _root{nullptr}, _size{0U}, _allocator{} {
			}

			/* @DOC Empty, drawing nodes from allocator. */
			constexpr explicit map(const A& allocator) noexcept
			: _root{nullptr}, _size{0U}, _allocator{allocator} {
			}

			map(const self&) = delete;

			constexpr map(self&& other) noexcept
			: _root{ca::exchange(other._root, nullptr)}, _size{ca::exchange(other._size, 0U)}, _allocator{other._allocator} {
			}

			// Failsafe for a skipped cleanup(): destroys the entries and frees, silently.
			// ~map(void) noexcept {
			// 	this->_release();
			// }

			/* @DOC dict.fromkeys: each key of keys mapped to a copy of value; later duplicates
			   change nothing. On failure the partial map is destroyed and that error returned. */
			[[nodiscard]] static auto from_keys(const ca::borrowed_slice<const K> keys, const V& value) noexcept -> ca::result<self>
			requires ca::is_stateless_allocator<A> && ca::impl::is_buildable_from<K, const K&> && ca::impl::is_buildable_from<V, const V&> {

				self made{};
				for (const K& key : keys) {
					if (made.contains(key))
						continue;
					auto added = made.set(key, value);
					if (!added) {
						static_cast<void>(made.cleanup());
						return added.propagate();
					}
				}
				return made;
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Takes other's entries and allocator; the previous entries are released as by the
			   destructor. Traps when an entry is cleanable and entries remain: clear() or cleanup()
			   first. */
			auto operator=(self&& other) noexcept -> self& {

				if (this == &other)
					return *this;
				if constexpr (ca::is_cleanable<entry>) {
					if (_size != 0U) [[unlikely]]
						ca::impl::assigned_over_uncleaned_elements();
				}
				this->_release();
				     _root = ca::exchange(other._root, nullptr);
				     _size = ca::exchange(other._size, 0U);
				_allocator = other._allocator;
				return *this;
			}


			// -- iterators --------------------------------------------------

			/* @DOC Every entry once, as item<const K&, V&>, in key order. */
			[[nodiscard]] auto begin(void) noexcept -> iterator {
				return iterator{self::_leftmost(_root), 0U};
			}

			[[nodiscard]] auto begin(void) const noexcept -> const_iterator {
				return const_iterator{self::_leftmost(_root), 0U};
			}

			[[nodiscard]] auto end(void) noexcept -> iterator {
				return iterator{nullptr, 0U};
			}

			[[nodiscard]] auto end(void) const noexcept -> const_iterator {
				return const_iterator{nullptr, 0U};
			}

			/* @DOC dict.keys: every key in order, read-only. */
			[[nodiscard]] auto keys(void) const noexcept -> ca::impl::range<key_iterator> {
				return ca::impl::range<key_iterator>{key_iterator{self::_leftmost(_root), 0U}, key_iterator{nullptr, 0U}};
			}

			/* @DOC dict.values: every value, in key order. */
			[[nodiscard]] auto values(void) noexcept -> ca::impl::range<value_iterator> {
				return ca::impl::range<value_iterator>{value_iterator{self::_leftmost(_root), 0U}, value_iterator{nullptr, 0U}};
			}

			[[nodiscard]] auto values(void) const noexcept -> ca::impl::range<const_value_iterator> {
				return ca::impl::range<const_value_iterator>{const_value_iterator{self::_leftmost(_root), 0U}, const_value_iterator{nullptr, 0U}};
			}

			/* @DOC dict.items: every entry in order, as item<const K&, V&>. */
			[[nodiscard]] auto items(void) noexcept -> ca::impl::range<iterator> {
				return ca::impl::range<iterator>{this->begin(), this->end()};
			}

			[[nodiscard]] auto items(void) const noexcept -> ca::impl::range<const_iterator> {
				return ca::impl::range<const_iterator>{this->begin(), this->end()};
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
				return _size;
			}

			[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
				return _size == 0U;
			}

			[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
				return _allocator;
			}


			// -- element access ---------------------------------------------

			/* @DOC The value of key. Traps when key is absent, in every build: Python's KeyError. */
			[[nodiscard]] auto operator[](const lookup_type key) noexcept -> V& {
				return self::_expect(this->_search(key));
			}

			[[nodiscard]] auto operator[](const lookup_type key) const noexcept -> const V& {
				return self::_expect(this->_search(key));
			}

			/* @DOC The value of key, or nothing when absent: dict.get. */
			[[nodiscard]] auto get(const lookup_type key) noexcept -> ca::optional<V&> {

				const position at = this->_search(key);
				if (!at._found)
					return ca::nullopt;
				return ca::optional<V&>{at._node->_values[at._index]};
			}

			[[nodiscard]] auto get(const lookup_type key) const noexcept -> ca::optional<const V&> {

				const position at = this->_search(key);
				if (!at._found)
					return ca::nullopt;
				return ca::optional<const V&>{at._node->_values[at._index]};
			}

			/* @DOC key in dict. */
			[[nodiscard]] auto contains(const lookup_type key) const noexcept -> bool {
				return this->_search(key)._found;
			}

			/* @DOC The entry with the smallest key, or nothing when empty. */
			[[nodiscard]] auto first(void) noexcept -> ca::optional<ca::item<const K&, V&>> {
				return self::_item_at<V>(self::_leftmost(_root), 0U);
			}

			[[nodiscard]] auto first(void) const noexcept -> ca::optional<ca::item<const K&, const V&>> {
				return self::_item_at<const V>(self::_leftmost(_root), 0U);
			}

			/* @DOC The entry with the largest key, or nothing when empty. */
			[[nodiscard]] auto last(void) noexcept -> ca::optional<ca::item<const K&, V&>> {

				node* const at = self::_rightmost(_root);
				return self::_item_at<V>(at, at == nullptr ? 0U : at->_size - 1U);
			}

			[[nodiscard]] auto last(void) const noexcept -> ca::optional<ca::item<const K&, const V&>> {

				node* const at = self::_rightmost(_root);
				return self::_item_at<const V>(at, at == nullptr ? 0U : at->_size - 1U);
			}

			/* @DOC The entry with the largest key not above key, or nothing. */
			[[nodiscard]] auto floor(const lookup_type key) noexcept -> ca::optional<ca::item<const K&, V&>> {

				const position at = this->_floor(key);
				return self::_item_at<V>(at._node, at._index);
			}

			[[nodiscard]] auto floor(const lookup_type key) const noexcept -> ca::optional<ca::item<const K&, const V&>> {

				const position at = this->_floor(key);
				return self::_item_at<const V>(at._node, at._index);
			}

			/* @DOC The entry with the smallest key not below key, or nothing. */
			[[nodiscard]] auto ceiling(const lookup_type key) noexcept -> ca::optional<ca::item<const K&, V&>> {

				const position at = this->_ceiling(key);
				return self::_item_at<V>(at._node, at._index);
			}

			[[nodiscard]] auto ceiling(const lookup_type key) const noexcept -> ca::optional<ca::item<const K&, const V&>> {

				const position at = this->_ceiling(key);
				return self::_item_at<const V>(at._node, at._index);
			}


			// -- modifiers --------------------------------------------------

			/* @DOC d[key] = value: maps key to a V built from value, replacing (cleanup, then
			   destructor) the previous one. The key is built from key only when absent. On failure
			   nothing changes; a cleanup error of the replaced value is returned once it is gone. */
			template <typename Q, typename U>
			requires ca::impl::is_buildable_from<K, Q> && ca::impl::is_buildable_from<V, U>
			[[nodiscard]] auto set(Q&& key, U&& value) noexcept -> ca::result<void> {

				const position at = this->_search(key);
				if (at._found) {
					auto made = ca::impl::make_from<V>(ca::forward<U>(value));
					if (!made)
						return made.propagate();
					V& old = at._node->_values[at._index];
					ca::error first{};
					ca::impl::clean(old, first);
					ca::destroy_at(&old);
					ca::construct_at(&old, ca::move(*made));
					if (!first.ok())
						return ca::failure{first};
					return ca::result<void>{};
				}
				auto added = this->_add(at, ca::forward<Q>(key), ca::forward<U>(value));
				if (!added)
					return added.propagate();
				return ca::result<void>{};
			}

			/* @DOC dict.setdefault: the value of key, mapping it first to a V built from fallback
			   when absent. On failure nothing changes. */
			template <typename Q, typename U>
			requires ca::impl::is_buildable_from<K, Q> && ca::impl::is_buildable_from<V, U>
			[[nodiscard]] auto setdefault(Q&& key, U&& fallback) noexcept -> ca::result<V*> {

				const position at = this->_search(key);
				if (at._found)
					return &at._node->_values[at._index];
				return this->_add(at, ca::forward<Q>(key), ca::forward<U>(fallback));
			}

			/* @DOC Removes key and returns its value, or nothing when absent: dict.pop. */
			[[nodiscard]] auto pop(const lookup_type key) noexcept -> ca::optional<V> requires ca::is_move_constructible<K> && ca::is_move_constructible<V> {

				const position at = this->_search(key);
				if (!at._found)
					return ca::nullopt;
				entry taken = this->_erase_at(at._node, at._index);
				ca::error ignored{};
				ca::impl::clean(taken.key, ignored);
				return ca::optional<V>{ca::in_place, ca::move(taken.value)};
			}

			/* @DOC Removes and returns the entry with the largest key, or nothing when empty:
			   dict.popitem, last in, here last in order. */
			[[nodiscard]] auto popitem(void) noexcept -> ca::optional<entry> requires ca::is_move_constructible<K> && ca::is_move_constructible<V> {

				node* const at = self::_rightmost(_root);
				if (at == nullptr)
					return ca::nullopt;
				return ca::optional<entry>{ca::in_place, this->_erase_at(at, at->_size - 1U)};
			}

			/* @DOC Removes and returns the entry with the smallest key, or nothing when empty. */
			[[nodiscard]] auto pop_first(void) noexcept -> ca::optional<entry> requires ca::is_move_constructible<K> && ca::is_move_constructible<V> {

				node* const at = self::_leftmost(_root);
				if (at == nullptr)
					return ca::nullopt;
				return ca::optional<entry>{ca::in_place, this->_erase_at(at, 0U)};
			}

			/* @DOC dict.update: copies every entry of other in, replacing the values of keys already
			   present. Stops at the first failure, the entries copied so far kept. */
			[[nodiscard]] auto update(const self& other) noexcept -> ca::result<void>
			requires ca::impl::is_buildable_from<K, const K&> && ca::impl::is_buildable_from<V, const V&> {

				if (this == &other)
					return ca::result<void>{};
				for (const auto [key, value] : other) {
					auto added = this->set(key, value);
					if (!added)
						return added;
				}
				return ca::result<void>{};
			}

			/* @DOC update, moving other's entries in; other is left empty. On failure the entries
			   not moved yet are dropped with other's storage. */
			[[nodiscard]] auto update(self&& other) noexcept -> ca::result<void> requires ca::is_move_constructible<K> && ca::is_move_constructible<V> {

				if (this == &other)
					return ca::result<void>{};
				ca::result<void> outcome{};
				for (auto [key, value] : other) {
					outcome = this->set(ca::move(const_cast<K&>(key)), ca::move(value));
					if (!outcome)
						break;
				}
				static_cast<void>(other.cleanup());
				return outcome;
			}

			/* @DOC Keeps the entries for which test(key, value) holds; the others are cleaned up and
			   destroyed. Returns the first cleanup error; they are gone regardless. */
			template <typename F>
			[[nodiscard]] auto retain(F&& test) noexcept -> ca::result<void> requires ca::is_move_constructible<K> && ca::is_move_constructible<V> {

				ca::error first{};
				position at{self::_leftmost(_root), 0U, false};
				while (at._node != nullptr) {
					if (test(static_cast<const K&>(at._node->_keys[at._index]), at._node->_values[at._index])) {
						at = self::_next(at);
						continue;
					}
					entry taken = this->_erase_at(at._node, at._index);
					at = this->_ceiling(ca::impl::key_view(taken.key));
					ca::impl::clean(taken.key, first);
					ca::impl::clean(taken.value, first);
				}
				if (!first.ok())
					return ca::failure{first};
				return ca::result<void>{};
			}

			/* @DOC Cleans up, destroys and frees every entry and node. Returns the first cleanup
			   error; every entry is gone regardless. */
			[[nodiscard]] auto clear(void) noexcept -> ca::result<void> {

				ca::error first{};
				if (_root != nullptr)
					self::_clear_subtree(_allocator, _root, first);
				_root = nullptr;
				_size = 0U;
				if (!first.ok())
					return ca::failure{first};
				return ca::result<void>{};
			}

			/* @DOC clear(): a map keeps no memory beyond its nodes. */
			[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {
				return this->clear();
			}


			// -- methods ----------------------------------------------------

			/* @DOC Same keys mapped to equal values (V's ==). */
			[[nodiscard]] auto operator==(const self& other) const noexcept -> bool {

				if (_size != other._size)
					return false;
				auto right = other.begin();
				for (const auto [key, value] : *this) {
					const auto [other_key, other_value] = *right;
					if (ca::impl::key_order(ca::impl::key_view(key), ca::impl::key_view(other_key)) != 0 || !(value == other_value))
						return false;
					++right;
				}
				return true;
			}

			/* @DOC A copy with nodes from the same allocator, same shape: dict.copy. Keys and values
			   are copied with clone() when they have one, else with the copy constructor. On a
			   failure, the partial copy is destroyed and that error returned. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self>
			requires (ca::is_clonable<K> || ca::is_copy_constructible<K>) && (ca::is_clonable<V> || ca::is_copy_constructible<V>) {
				return this->clone_in(_allocator);
			}

			/* @DOC clone, with nodes from allocator. */
			[[nodiscard]] auto clone_in(const A& allocator) const noexcept -> ca::result<self>
			requires (ca::is_clonable<K> || ca::is_copy_constructible<K>) && (ca::is_clonable<V> || ca::is_copy_constructible<V>) {

				self copy{allocator};
				if (_root == nullptr)
					return copy;
				auto root = self::_cloned(allocator, _root, nullptr, 0U);
				if (!root)
					return root.propagate();
				copy._root = *root;
				copy._size = _size;
				return copy;
			}


		private:

			// -- private methods --------------------------------------------

			[[nodiscard, gnu::always_inline]] static auto _children(node* const at) noexcept -> node** {
				return static_cast<branch*>(at)->_children;
			}

			[[nodiscard]] static auto _leftmost(node* at) noexcept -> node* {

				if (at == nullptr)
					return nullptr;
				while (at->_branch)
					at = self::_children(at)[0];
				return at;
			}

			[[nodiscard]] static auto _rightmost(node* at) noexcept -> node* {

				if (at == nullptr)
					return nullptr;
				while (at->_branch)
					at = self::_children(at)[at->_size];
				return at;
			}

			template <typename W>
			[[nodiscard]] static auto _item_at(node* const at, const ca::usz index) noexcept -> ca::optional<ca::item<const K&, W&>> {

				if (at == nullptr)
					return ca::nullopt;
				return ca::optional<ca::item<const K&, W&>>{ca::in_place, at->_keys[index], at->_values[index]};
			}

			[[nodiscard]] static auto _next(const position at) noexcept -> position {

				node* walk = at._node;
				ca::usz index = at._index;
				if (walk->_branch) {
					walk = self::_children(walk)[index + 1U];
					while (walk->_branch)
						walk = self::_children(walk)[0];
					return position{walk, 0U, false};
				}
				++index;
				while (index == walk->_size) {
					if (walk->_parent == nullptr)
						return position{nullptr, 0U, false};
					index = walk->_index;
					 walk = walk->_parent;
				}
				return position{walk, index, false};
			}

			[[nodiscard]] static auto _expect(const position at) noexcept -> V& {

				if (!at._found) [[unlikely]]
					ca::impl::key_not_found();
				return at._node->_values[at._index];
			}

			[[nodiscard]] auto _search(const lookup_type key) const noexcept -> position {

				node* at = _root;
				if (at == nullptr)
					return position{nullptr, 0U, false};
				for (;;) {
					const ca::usz size = at->_size;
					ca::usz index = 0U;
					for (; index < size; ++index) {
						const int order = ca::impl::key_order(key, ca::impl::key_view(at->_keys[index]));
						if (order == 0)
							return position{at, index, true};
						if (order < 0)
							break;
					}
					if (!at->_branch)
						return position{at, index, false};
					at = self::_children(at)[index];
				}
			}

			[[nodiscard]] auto _ceiling(const lookup_type key) const noexcept -> position {

				position best{nullptr, 0U, false};
				for (node* at = _root; at != nullptr;) {
					ca::usz index = 0U;
					for (; index < at->_size; ++index) {
						const int order = ca::impl::key_order(key, ca::impl::key_view(at->_keys[index]));
						if (order == 0)
							return position{at, index, true};
						if (order < 0)
							break;
					}
					if (index < at->_size)
						best = position{at, index, false};
					at = at->_branch ? self::_children(at)[index] : nullptr;
				}
				return best;
			}

			[[nodiscard]] auto _floor(const lookup_type key) const noexcept -> position {

				position best{nullptr, 0U, false};
				for (node* at = _root; at != nullptr;) {
					ca::usz index = 0U;
					for (; index < at->_size; ++index) {
						const int order = ca::impl::key_order(key, ca::impl::key_view(at->_keys[index]));
						if (order == 0)
							return position{at, index, true};
						if (order < 0)
							break;
					}
					if (index > 0U)
						best = position{at, index - 1U, false};
					at = at->_branch ? self::_children(at)[index] : nullptr;
				}
				return best;
			}

			[[nodiscard]] static auto _new_node(const A allocator, const bool branched) noexcept -> ca::result<node*> {

				if (branched) {
					auto block = ca::allocate_n<branch>(allocator, 1U);
					if (!block) [[unlikely]]
						return block.propagate();
					return static_cast<node*>(ca::construct_at(static_cast<branch*>(block->data())));
				}
				auto block = ca::allocate_n<node>(allocator, 1U);
				if (!block) [[unlikely]]
					return block.propagate();
				return ca::construct_at(static_cast<node*>(block->data()), false);
			}

			static auto _free_node(const A allocator, node* const at) noexcept -> void {
				if (at->_branch) {
					branch* const full = static_cast<branch*>(at);
					ca::destroy_at(full);
					ca::deallocate_n<branch>(allocator, ca::allocation{full, sizeof(branch)});
				}
				else {
					ca::destroy_at(at);
					ca::deallocate_n<node>(allocator, ca::allocation{at, sizeof(node)});
				}
			}

			static auto _release_subtree(const A allocator, node* const at) noexcept -> void {

				ca::destroy_n(at->_keys, at->_size);
				ca::destroy_n(at->_values, at->_size);
				if (at->_branch) {
					for (ca::usz index = 0U; index <= at->_size; ++index)
						self::_release_subtree(allocator, self::_children(at)[index]);
				}
				self::_free_node(allocator, at);
			}

			static auto _clean_entries(node* const at, ca::error& first) noexcept -> void {

				for (ca::usz index = 0U; index < at->_size; ++index) {
					ca::impl::clean(at->_keys[index], first);
					ca::impl::clean(at->_values[index], first);
				}
				ca::destroy_n(at->_keys, at->_size);
				ca::destroy_n(at->_values, at->_size);
			}

			static auto _clear_subtree(const A allocator, node* const at, ca::error& first) noexcept -> void {

				if (at->_branch) {
					for (ca::usz index = 0U; index <= at->_size; ++index)
						self::_clear_subtree(allocator, self::_children(at)[index], first);
				}
				self::_clean_entries(at, first);
				self::_free_node(allocator, at);
			}

			[[nodiscard]] static auto _cloned(const A allocator, node* const from, node* const parent, const ca::usz index) noexcept -> ca::result<node*> {

				auto made = self::_new_node(allocator, from->_branch);
				if (!made)
					return made;
				node* const to = *made;
				to->_parent = parent;
				 to->_index = static_cast<ca::u16>(index);
				if constexpr (ca::is_trivially_copyable<K> && ca::is_trivially_copyable<V> && !ca::is_clonable<K> && !ca::is_clonable<V>) {
					ca::copy_bytes(to->_keys, from->_keys, from->_size * sizeof(K));
					ca::copy_bytes(to->_values, from->_values, from->_size * sizeof(V));
					to->_size = from->_size;
				}
				else {
					for (ca::usz at = 0U; at < from->_size; ++at) {
						auto key = ca::impl::make_clone(from->_keys[at]);
						if (!key) {
							self::_discard(allocator, to, 0U);
							return key.propagate();
						}
						auto value = ca::impl::make_clone(from->_values[at]);
						if (!value) {
							ca::error ignored{};
							ca::impl::clean(*key, ignored);
							self::_discard(allocator, to, 0U);
							return value.propagate();
						}
						ca::construct_at(to->_keys + at, ca::move(*key));
						ca::construct_at(to->_values + at, ca::move(*value));
						++to->_size;
					}
				}
				if (from->_branch) {
					for (ca::usz child = 0U; child <= from->_size; ++child) {
						auto cloned = self::_cloned(allocator, self::_children(from)[child], to, child);
						if (!cloned) {
							self::_discard(allocator, to, child);
							return cloned;
						}
						self::_children(to)[child] = *cloned;
					}
				}
				return to;
			}

			static auto _discard(const A allocator, node* const at, const ca::usz children) noexcept -> void {

				ca::error ignored{};
				for (ca::usz child = 0U; child < children; ++child)
					self::_clear_subtree(allocator, self::_children(at)[child], ignored);
				self::_clean_entries(at, ignored);
				self::_free_node(allocator, at);
			}

			template <typename Q, typename U>
			[[nodiscard]] auto _add(position at, Q&& key, U&& value) noexcept -> ca::result<V*> {

				auto made_key = ca::impl::make_from<K>(ca::forward<Q>(key));
				if (!made_key)
					return made_key.propagate();
				auto made_value = ca::impl::make_from<V>(ca::forward<U>(value));
				if (!made_value) {
					ca::error ignored{};
					ca::impl::clean(*made_key, ignored);
					return made_value.propagate();
				}
				auto added = this->_insert(at, *made_key, *made_value);
				if (!added) {
					ca::error ignored{};
					ca::impl::clean(*made_key, ignored);
					ca::impl::clean(*made_value, ignored);
				}
				return added;
			}

			[[nodiscard]] auto _insert(position at, K& key, V& value) noexcept -> ca::result<V*> {

				if (_root == nullptr) {
					auto made = self::_new_node(_allocator, false);
					if (!made)
						return made.propagate();
					_root = *made;
					   at = position{_root, 0U, false};
				}
				ca::usz needed = 0U;
				node* full = at._node;
				while (full != nullptr && full->_size == self::_capacity) {
					++needed;
					full = full->_parent;
				}
				if (full == nullptr && needed != 0U)
					++needed;
				node* spare[self::_max_height + 1U];
				for (ca::usz made_count = 0U; made_count < needed; ++made_count) {
					auto made = self::_new_node(_allocator, made_count != 0U);
					if (!made) {
						for (ca::usz index = 0U; index < made_count; ++index)
							self::_free_node(_allocator, spare[index]);
						return made.propagate();
					}
					spare[made_count] = *made;
				}
				node* target = at._node;
				const ca::usz index = at._index;
				ca::relocate_overlapping_n(target->_keys + index, target->_size - index, target->_keys + index + 1U);
				ca::relocate_overlapping_n(target->_values + index, target->_size - index, target->_values + index + 1U);
				ca::construct_at(target->_keys + index, ca::move(key));
				ca::construct_at(target->_values + index, ca::move(value));
				++target->_size;
				++_size;
				node* held = target;
				ca::usz held_index = index;
				ca::usz used = 0U;
				while (target->_size > self::_capacity) {
					node* const right = spare[used++];
					const ca::usz keep = self::_minimum + 1U;
					const ca::usz moved = target->_size - keep - 1U;
					ca::relocate_n(target->_keys + keep + 1U, moved, right->_keys);
					ca::relocate_n(target->_values + keep + 1U, moved, right->_values);
					if (target->_branch) {
						for (ca::usz child = 0U; child <= moved; ++child)
							self::_adopt(right, child, self::_children(target)[keep + 1U + child]);
					}
					right->_size = static_cast<ca::u16>(moved);
					node* parent = target->_parent;
					if (parent == nullptr) {
						parent = spare[used++];
						self::_adopt(parent, 0U, target);
						_root = parent;
					}
					const ca::usz slot = target->_index;
					ca::relocate_overlapping_n(parent->_keys + slot, parent->_size - slot, parent->_keys + slot + 1U);
					ca::relocate_overlapping_n(parent->_values + slot, parent->_size - slot, parent->_values + slot + 1U);
					for (ca::usz child = parent->_size; child > slot; --child)
						self::_adopt(parent, child + 1U, self::_children(parent)[child]);
					ca::relocate_n(target->_keys + keep, 1U, parent->_keys + slot);
					ca::relocate_n(target->_values + keep, 1U, parent->_values + slot);
					target->_size = static_cast<ca::u16>(keep);
					self::_adopt(parent, slot + 1U, right);
					++parent->_size;
					if (held == target) {
						if (held_index == keep) {
							      held = parent;
							held_index = slot;
						}
						else if (held_index > keep) {
							      held = right;
							held_index -= keep + 1U;
						}
					}
					target = parent;
				}
				return &held->_values[held_index];
			}

			[[gnu::always_inline]] static auto _adopt(node* const parent, const ca::usz index, node* const child) noexcept -> void {

				self::_children(parent)[index] = child;
				child->_parent = parent;
				 child->_index = static_cast<ca::u16>(index);
			}

			[[nodiscard]] auto _erase_at(node* at, const ca::usz index) noexcept -> entry {

				entry taken{ca::move(at->_keys[index]), ca::move(at->_values[index])};
				ca::destroy_at(at->_keys + index);
				ca::destroy_at(at->_values + index);
				if (at->_branch) {
					node* leaf = self::_children(at)[index];
					while (leaf->_branch)
						leaf = self::_children(leaf)[leaf->_size];
					const ca::usz last = leaf->_size - 1U;
					ca::relocate_n(leaf->_keys + last, 1U, at->_keys + index);
					ca::relocate_n(leaf->_values + last, 1U, at->_values + index);
					--leaf->_size;
					at = leaf;
				}
				else {
					ca::relocate_overlapping_n(at->_keys + index + 1U, at->_size - index - 1U, at->_keys + index);
					ca::relocate_overlapping_n(at->_values + index + 1U, at->_size - index - 1U, at->_values + index);
					--at->_size;
				}
				--_size;
				this->_rebalance(at);
				return taken;
			}

			auto _rebalance(node* at) noexcept -> void {

				while (at != _root && at->_size < self::_minimum) {
					node* const parent = at->_parent;
					const ca::usz index = at->_index;
					node* const left = index > 0U ? self::_children(parent)[index - 1U] : nullptr;
					node* const right = index < parent->_size ? self::_children(parent)[index + 1U] : nullptr;
					if (left != nullptr && left->_size > self::_minimum) {
						self::_rotate_right(parent, index - 1U, left, at);
						return;
					}
					if (right != nullptr && right->_size > self::_minimum) {
						self::_rotate_left(parent, index, at, right);
						return;
					}
					if (left != nullptr)
						this->_merge(parent, index - 1U, left, at);
					else
						this->_merge(parent, index, at, right);
					at = parent;
				}
				if (_root->_size != 0U)
					return;
				node* const old = _root;
				if (old->_branch) {
					         _root = self::_children(old)[0];
					_root->_parent = nullptr;
					 _root->_index = 0U;
				}
				else
					_root = nullptr;
				self::_free_node(_allocator, old);
			}

			static auto _rotate_right(node* const parent, const ca::usz separator, node* const left, node* const at) noexcept -> void {

				ca::relocate_overlapping_n(at->_keys, at->_size, at->_keys + 1U);
				ca::relocate_overlapping_n(at->_values, at->_size, at->_values + 1U);
				ca::relocate_n(parent->_keys + separator, 1U, at->_keys);
				ca::relocate_n(parent->_values + separator, 1U, at->_values);
				ca::relocate_n(left->_keys + left->_size - 1U, 1U, parent->_keys + separator);
				ca::relocate_n(left->_values + left->_size - 1U, 1U, parent->_values + separator);
				if (at->_branch) {
					for (ca::usz child = at->_size + 1U; child > 0U; --child)
						self::_adopt(at, child, self::_children(at)[child - 1U]);
					self::_adopt(at, 0U, self::_children(left)[left->_size]);
				}
				--left->_size;
				++at->_size;
			}

			static auto _rotate_left(node* const parent, const ca::usz separator, node* const at, node* const right) noexcept -> void {

				ca::relocate_n(parent->_keys + separator, 1U, at->_keys + at->_size);
				ca::relocate_n(parent->_values + separator, 1U, at->_values + at->_size);
				ca::relocate_n(right->_keys, 1U, parent->_keys + separator);
				ca::relocate_n(right->_values, 1U, parent->_values + separator);
				ca::relocate_overlapping_n(right->_keys + 1U, right->_size - 1U, right->_keys);
				ca::relocate_overlapping_n(right->_values + 1U, right->_size - 1U, right->_values);
				if (at->_branch) {
					self::_adopt(at, at->_size + 1U, self::_children(right)[0]);
					for (ca::usz child = 0U; child < right->_size; ++child)
						self::_adopt(right, child, self::_children(right)[child + 1U]);
				}
				++at->_size;
				--right->_size;
			}

			auto _merge(node* const parent, const ca::usz separator, node* const left, node* const right) noexcept -> void {

				const ca::usz left_size = left->_size;
				const ca::usz right_size = right->_size;
				ca::relocate_n(parent->_keys + separator, 1U, left->_keys + left_size);
				ca::relocate_n(parent->_values + separator, 1U, left->_values + left_size);
				ca::relocate_n(right->_keys, right_size, left->_keys + left_size + 1U);
				ca::relocate_n(right->_values, right_size, left->_values + left_size + 1U);
				if (left->_branch) {
					for (ca::usz child = 0U; child <= right_size; ++child)
						self::_adopt(left, left_size + 1U + child, self::_children(right)[child]);
				}
				left->_size = static_cast<ca::u16>(left_size + 1U + right_size);
				ca::relocate_overlapping_n(parent->_keys + separator + 1U, parent->_size - separator - 1U, parent->_keys + separator);
				ca::relocate_overlapping_n(parent->_values + separator + 1U, parent->_size - separator - 1U, parent->_values + separator);
				for (ca::usz child = separator + 1U; child < parent->_size; ++child)
					self::_adopt(parent, child, self::_children(parent)[child + 1U]);
				--parent->_size;
				right->_size = 0U;
				self::_free_node(_allocator, right);
			}

			auto _release(void) noexcept -> void {

				if (_root != nullptr)
					self::_release_subtree(_allocator, _root);
				_root = nullptr;
				_size = 0U;
			}


			// -- assertions -------------------------------------------------

			static_assert(ca::is_object<K> && !ca::is_const<K> && !ca::is_array<K> && ca::is_object<V> && !ca::is_const<V> && !ca::is_array<V>,
				"ca::map<K, V>: K and V must be non-const, non-array object types");

	}; // class map

} // namespace ca

#endif // ca_collections_map_map_hpp
