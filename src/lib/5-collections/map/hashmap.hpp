/* @DOC Performance of ca::hashmap<K, V, A>.

   Layout: control pointer, slot pointer, mask, size and growth left, 40 bytes on 64-bit, plus
   the allocator (nothing for ca::heap). One block holds a control byte per slot, then the slots,
   each an item<K, V> (the key and the value side by side: one cache line serves the compare and
   the value). An empty hashmap owns no memory: it points at a shared group of EMPTY control
   bytes, so lookups take the same path with or without storage.

   | Operation | Time | Notes |
   |---|---|---|
   | get, contains, operator[] | O(1) expected | hash, one 8-byte control load and match per group probed, one key compare per tag hit (1/128 false hits) |
   | set, setdefault | amortized O(1) expected | the lookup, then an insert into the first free slot met |
   | pop(key) | O(1) expected | marks the slot EMPTY when its group has one, else DELETED |
   | popitem | O(capacity / 8) worst | scans control groups from the start |
   | iteration, keys, values, items | O(capacity / 8 + size) | 8 control bytes per step |
   | reserve, shrink, growth | O(capacity) | every slot rehashed and relocated (memcpy for trivially relocatable items) |
   | clear, destroy | O(capacity / 8 + size) | no walk for trivially destructible items without destroy() |
   | clone | O(capacity) | same placement, no rehash; one memcpy for trivially copyable items |
   | update | O(other.size) | |

   Space: capacity is a power of two from 8, at most 7/8 of it used, so 1 + sizeof(item) bytes
   per slot at 44% to 87.5% load right after growth. Growth doubles, or rehashes in place when
   DELETED slots, not elements, filled the table.

   Locality: a lookup touches one control group (8 bytes) and, on a tag hit, one slot; misses
   rarely touch a slot. Probing walks aligned groups triangularly, so it visits every group.

   Hashing: ca::hash, wyhash on bytes, one 128-bit multiply on integers. Keys are looked up by
   lookup_type: an sv for str keys, so literals and views never build a string.

   Relocation: TRIVIALLY_RELOCATABLE when A is; slots relocate with memcpy on growth when the
   item is trivially relocatable (str keys included).

   Copy and move: the copy constructor is deleted; clone() is the explicit, fallible copy. Move
   takes five words and the allocator, no allocation.

   Inlining: get and contains inline to the probe loop; growth and rehash are out of line and
   take the allocator by value, not the map.

   Baseline std::unordered_map over glibc malloc; ca over ca::heap; u64 keys and values unless
   said:
   --BENCH bench/hashmap.cpp
   bench/hashmap.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | insert 1e6 scrambled u64, growing, then destroy | 0.15x | 0.15x | |
   | 1e7 lookup hits in 1e6 entries | 0.46x | 0.43x | |
   | 1e7 lookup misses | 0.098x | 0.096x | misses rarely touch a slot |
   | walk 1e6 entries 10 times | 0.20x | 0.21x | |
   | insert + pop churn, 1e5 live, 5e6 rounds | 0.33x | 0.35x | DELETED reclaimed by in-place rehash |
   | 1e5 maps of 8 ints, built, read, destroyed | 0.88x | 0.50x | |
   | str keys, 1e5 inserted, 1e6 lookups by literal | 0.27x | 0.31x | std: std::string keys |
   BENCH-- */

#ifndef ca_collections_map_hashmap_hpp
#define ca_collections_map_hashmap_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_move_constructible.hpp"
#include "1-primitives/traits/is_object.hpp"
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
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "5-collections/map/is_hashable.hpp"
#include "5-collections/map/item.hpp"
#include "5-collections/map/impl/hash_table.hpp"
#include "5-collections/map/impl/key.hpp"
#include "5-collections/map/impl/range.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- HASHMAP ------------------------------------------------------------

	namespace impl {

		/* @DOC Shows a hashmap slot as item<const K&, V&>. */
		struct project_item final {

				using self = ca::impl::project_item;

				template <typename K, typename V>
				[[nodiscard]] static constexpr auto get(ca::item<K, V>& slot) noexcept -> ca::item<const K&, V&> {
					return ca::item<const K&, V&>{slot.key, slot.value};
				}

				template <typename K, typename V>
				[[nodiscard]] static constexpr auto get(const ca::item<K, V>& slot) noexcept -> ca::item<const K&, const V&> {
					return ca::item<const K&, const V&>{slot.key, slot.value};
				}

		}; // struct project_item

		/* @DOC Shows a hashmap slot as its key. */
		struct project_key final {

				using self = ca::impl::project_key;

				template <typename K, typename V>
				[[nodiscard]] static constexpr auto get(const ca::item<K, V>& slot) noexcept -> const K& {
					return slot.key;
				}

		}; // struct project_key

		/* @DOC Shows a hashmap slot as its value. */
		struct project_value final {

				using self = ca::impl::project_value;

				template <typename K, typename V>
				[[nodiscard]] static constexpr auto get(ca::item<K, V>& slot) noexcept -> V& {
					return slot.value;
				}

				template <typename K, typename V>
				[[nodiscard]] static constexpr auto get(const ca::item<K, V>& slot) noexcept -> const V& {
					return slot.value;
				}

		}; // struct project_value

	} // namespace impl


	/* @DOC Unordered map from K to V in memory from A, Python's dict API over a Swiss table:
	   map[key] (traps when absent), get, set (d[key] = value), setdefault, pop, popitem, update,
	   keys, values, items, from_keys, contains (in), size (len), clear; clone is dict.copy.
	   Order is the table's, not insertion order. Keys are looked up by lookup_type (an sv for str
	   keys); set and setdefault build the key from what they are given only when it is absent
	   (str keys from literals or views). K is ca::hash-able and compares with == against its
	   lookup type. Construction allocates nothing; set, setdefault, update, reserve and clone
	   acquire memory and can fail, changing nothing. Insertions may move every entry: positions
	   and references do not survive them. Clean up with destroy(); the destructor is a failsafe
	   that frees silently. A stateful A has no default construction and moves with the
	   entries. */
	template <typename K, typename V, ca::is_allocator A = ca::heap>
	class hashmap final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::hashmap<K, V, A>;

			using entry = ca::item<K, V>;

			using table = ca::impl::hash_table<K, entry, A>;


		public:

			// -- public types -----------------------------------------------

			using key_type             = K;
			using mapped_type          = V;
			using value_type           = ca::item<K, V>;
			using lookup_type          = ca::impl::lookup_of<K>;
			using allocator_type       = A;
			using size_type            = ca::usz;
			using iterator             = ca::impl::table_iterator<table, entry, ca::impl::project_item>;
			using const_iterator       = ca::impl::table_iterator<table, const entry, ca::impl::project_item>;
			using key_iterator         = ca::impl::table_iterator<table, const entry, ca::impl::project_key>;
			using value_iterator       = ca::impl::table_iterator<table, entry, ca::impl::project_value>;
			using const_value_iterator = ca::impl::table_iterator<table, const entry, ca::impl::project_value>;

			/* @DOC The same map type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::hashmap<K, V, B>;


			// -- public constants -------------------------------------------

			/* @DOC No pointer into itself: containers move a hashmap with memcpy when A allows it. */
			static constexpr bool TRIVIALLY_RELOCATABLE = table::TRIVIALLY_RELOCATABLE;


		private:

			// -- private members --------------------------------------------

			table _table;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr hashmap(void) noexcept requires ca::is_stateless_allocator<A>
			: _table{} {
			}

			/* @DOC Empty, drawing memory from allocator. */
			constexpr explicit hashmap(const A& allocator) noexcept
			: _table{allocator} {
			}

			hashmap(const self&) = delete;

			constexpr hashmap(self&& other) noexcept
			: _table{ca::move(other._table)} {
			}

			/* @DOC Failsafe for a skipped destroy(): destroys the entries and frees, silently. */
			~hashmap(void) noexcept {
			}

			/* @DOC dict.fromkeys: each key of keys mapped to a copy of value; later duplicates
			   change nothing. On failure the partial map is destroyed and that error returned. */
			[[nodiscard]] static auto from_keys(const ca::borrowed_slice<const K> keys, const V& value) noexcept -> ca::result<self>
			requires ca::is_stateless_allocator<A> && ca::impl::is_buildable_from<K, const K&> && ca::impl::is_buildable_from<V, const V&> {

				self made{};
				auto reserved = made.reserve(keys.size());
				if (!reserved)
					return reserved.propagate();
				for (const K& key : keys) {
					if (made.contains(key))
						continue;
					auto added = made.set(key, value);
					if (!added) {
						static_cast<void>(made.destroy());
						return added.propagate();
					}
				}
				return made;
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Takes other's entries and allocator; the previous entries are released as by the
			   destructor. Traps when an entry is cleanable and entries remain: clear() or destroy()
			   first. */
			auto operator=(self&& other) noexcept -> self& {

				_table = ca::move(other._table);
				return *this;
			}


			// -- iterators --------------------------------------------------

			/* @DOC Every entry once, as item<const K&, V&>, in table order. */
			[[nodiscard]] auto begin(void) noexcept -> iterator {
				return iterator{&_table, _table.next(0U)};
			}

			[[nodiscard]] auto begin(void) const noexcept -> const_iterator {
				return const_iterator{&_table, _table.next(0U)};
			}

			[[nodiscard]] auto end(void) noexcept -> iterator {
				return iterator{&_table, _table.capacity()};
			}

			[[nodiscard]] auto end(void) const noexcept -> const_iterator {
				return const_iterator{&_table, _table.capacity()};
			}

			/* @DOC dict.keys: every key, read-only. */
			[[nodiscard]] auto keys(void) const noexcept -> ca::impl::range<key_iterator> {
				return ca::impl::range<key_iterator>{key_iterator{&_table, _table.next(0U)}, key_iterator{&_table, _table.capacity()}};
			}

			/* @DOC dict.values: every value. */
			[[nodiscard]] auto values(void) noexcept -> ca::impl::range<value_iterator> {
				return ca::impl::range<value_iterator>{value_iterator{&_table, _table.next(0U)}, value_iterator{&_table, _table.capacity()}};
			}

			[[nodiscard]] auto values(void) const noexcept -> ca::impl::range<const_value_iterator> {
				return ca::impl::range<const_value_iterator>{const_value_iterator{&_table, _table.next(0U)}, const_value_iterator{&_table, _table.capacity()}};
			}

			/* @DOC dict.items: every entry, as item<const K&, V&>; begin() and end() as a range. */
			[[nodiscard]] auto items(void) noexcept -> ca::impl::range<iterator> {
				return ca::impl::range<iterator>{this->begin(), this->end()};
			}

			[[nodiscard]] auto items(void) const noexcept -> ca::impl::range<const_iterator> {
				return ca::impl::range<const_iterator>{this->begin(), this->end()};
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
				return _table.size();
			}

			[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
				return _table.size() == 0U;
			}

			/* @DOC Slots in the storage, 7/8 of them usable; 0 when none is held. */
			[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
				return _table.capacity();
			}

			[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
				return _table.allocator();
			}


			// -- element access ---------------------------------------------

			/* @DOC The value of key. Traps when key is absent, in every build: Python's KeyError. */
			[[nodiscard]] auto operator[](const lookup_type key) noexcept -> V& {
				return self::_expect(_table.find(key))->value;
			}

			[[nodiscard]] auto operator[](const lookup_type key) const noexcept -> const V& {
				return self::_expect(_table.find(key))->value;
			}

			/* @DOC The value of key, or nothing when absent: dict.get (get(key).value_or(fallback)
			   for its default). */
			[[nodiscard]] auto get(const lookup_type key) noexcept -> ca::optional<V&> {

				entry* const found = _table.find(key);
				if (found == nullptr)
					return ca::nullopt;
				return ca::optional<V&>{found->value};
			}

			[[nodiscard]] auto get(const lookup_type key) const noexcept -> ca::optional<const V&> {

				const entry* const found = _table.find(key);
				if (found == nullptr)
					return ca::nullopt;
				return ca::optional<const V&>{found->value};
			}

			/* @DOC key in dict. */
			[[nodiscard]] auto contains(const lookup_type key) const noexcept -> bool {
				return _table.find(key) != nullptr;
			}


			// -- modifiers --------------------------------------------------

			/* @DOC d[key] = value: maps key to a V built from value, replacing (cleanup, then
			   destructor) the previous one. The key is built from key only when absent. On failure
			   nothing changes; a cleanup error of the replaced value is returned once it is gone. */
			template <typename Q, typename U>
			requires ca::impl::is_buildable_from<K, Q> && ca::impl::is_buildable_from<V, U>
			[[nodiscard]] auto set(Q&& key, U&& value) noexcept -> ca::result<void> {

				auto spot = _table.prepare(key);
				if (!spot)
					return spot.propagate();
				if (spot->_found) {
					auto made = ca::impl::make_from<V>(ca::forward<U>(value));
					if (!made)
						return made.propagate();
					V& old = _table.slot(spot->_index)->value;
					ca::error first{};
					ca::impl::clean(old, first);
					ca::destroy_at(&old);
					ca::construct_at(&old, ca::move(*made));
					if (!first.ok())
						return ca::failure{first};
					return ca::result<void>{};
				}
				return this->_insert(*spot, ca::forward<Q>(key), ca::forward<U>(value));
			}

			/* @DOC dict.setdefault: the value of key, mapping it first to a V built from fallback
			   when absent. On failure nothing changes. */
			template <typename Q, typename U>
			requires ca::impl::is_buildable_from<K, Q> && ca::impl::is_buildable_from<V, U>
			[[nodiscard]] auto setdefault(Q&& key, U&& fallback) noexcept -> ca::result<V*> {

				auto spot = _table.prepare(key);
				if (!spot)
					return spot.propagate();
				if (!spot->_found) {
					auto added = this->_insert(*spot, ca::forward<Q>(key), ca::forward<U>(fallback));
					if (!added)
						return added.propagate();
				}
				return &_table.slot(spot->_index)->value;
			}

			/* @DOC Removes key and returns its value, or nothing when absent: dict.pop (del d[key]
			   when discarded). */
			[[nodiscard]] auto pop(const lookup_type key) noexcept -> ca::optional<V> requires ca::is_move_constructible<V> {

				entry* const found = _table.find(key);
				if (found == nullptr)
					return ca::nullopt;
				ca::optional<V> taken{ca::in_place, ca::move(found->value)};
				ca::error ignored{};
				ca::impl::clean(found->key, ignored);
				this->_erase(found);
				return taken;
			}

			/* @DOC Removes and returns an entry, the first in table order, or nothing when empty:
			   dict.popitem. */
			[[nodiscard]] auto popitem(void) noexcept -> ca::optional<entry> requires ca::is_move_constructible<K> && ca::is_move_constructible<V> {

				const ca::usz index = _table.next(0U);
				if (index == _table.capacity())
					return ca::nullopt;
				entry* const found = _table.slot(index);
				ca::optional<entry> taken{ca::in_place, ca::move(found->key), ca::move(found->value)};
				this->_erase(found);
				return taken;
			}

			/* @DOC dict.update: copies every entry of other in, replacing the values of keys already
			   present. Stops at the first failure, the entries copied so far kept. */
			[[nodiscard]] auto update(const self& other) noexcept -> ca::result<void>
			requires ca::impl::is_buildable_from<K, const K&> && ca::impl::is_buildable_from<V, const V&> {

				if (this == &other)
					return ca::result<void>{};
				auto reserved = _table.reserve(this->size() + other.size());
				if (!reserved)
					return reserved.propagate();
				for (const auto [key, value] : other) {
					auto added = this->set(key, value);
					if (!added)
						return added;
				}
				return ca::result<void>{};
			}

			/* @DOC update, moving other's entries in; other is left empty. Stops at the first
			   failure, the entries not moved yet left in other. */
			[[nodiscard]] auto update(self&& other) noexcept -> ca::result<void>
			requires ca::is_move_constructible<K> && ca::is_move_constructible<V> {

				if (this == &other)
					return ca::result<void>{};
				auto reserved = _table.reserve(this->size() + other.size());
				if (!reserved)
					return reserved.propagate();
				for (ca::usz index = other._table.next(0U); index < other._table.capacity(); index = other._table.next(index + 1U)) {
					entry* const moved = other._table.slot(index);
					auto added = this->set(ca::move(moved->key), ca::move(moved->value));
					other._erase(moved);
					if (!added)
						return added;
				}
				return ca::result<void>{};
			}

			/* @DOC Keeps the entries for which test(key, value) holds; the others are cleaned up and
			   destroyed. Returns the first cleanup error; they are gone regardless. */
			template <typename F>
			[[nodiscard]] auto retain(F&& test) noexcept -> ca::result<void> {
				return _table.retain([&test](entry& slot) {
					return static_cast<bool>(test(static_cast<const K&>(slot.key), slot.value));
				});
			}

			/* @DOC Room for count entries in total without growing. On failure nothing changes. */
			[[nodiscard]] auto reserve(const ca::usz count) noexcept -> ca::result<void> {
				return _table.reserve(count);
			}

			/* @DOC Brings capacity down to what size() needs, dropping DELETED slots; frees the
			   storage when empty. On failure nothing changes. */
			[[nodiscard]] auto shrink(void) noexcept -> ca::result<void> {
				return _table.shrink();
			}

			/* @DOC Cleans up and destroys every entry; keeps the capacity. Returns the first cleanup
			   error; every entry is gone regardless. */
			[[nodiscard]] auto clear(void) noexcept -> ca::result<void> {
				return _table.clear();
			}

			/* @DOC clear(), then frees the storage. */
			[[nodiscard]] auto destroy(void) noexcept -> ca::result<void> {
				return _table.destroy();
			}


			// -- methods ----------------------------------------------------

			/* @DOC Same keys mapped to equal values (V's ==), in any order. */
			[[nodiscard]] auto operator==(const self& other) const noexcept -> bool {

				if (this->size() != other.size())
					return false;
				for (const auto [key, value] : *this) {
					const entry* const found = other._table.find(ca::impl::key_view(key));
					if (found == nullptr || !(found->value == value))
						return false;
				}
				return true;
			}

			/* @DOC A copy in memory from the same allocator, same capacity, entries in the same
			   slots: dict.copy. Keys and values are copied with clone() when they have one, else
			   with the copy constructor. On a failure, the partial copy is destroyed and that error
			   returned. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self> requires ca::is_clonable<entry> || ca::is_copy_constructible<entry> {
				return this->clone_in(_table.allocator());
			}

			/* @DOC clone, in memory from allocator. */
			[[nodiscard]] auto clone_in(const A& allocator) const noexcept -> ca::result<self> requires ca::is_clonable<entry> || ca::is_copy_constructible<entry> {

				auto copied = _table.clone_in(allocator);
				if (!copied)
					return copied.propagate();
				self copy{allocator};
				copy._table = ca::move(*copied);
				return copy;
			}


		private:

			// -- private methods --------------------------------------------

			[[nodiscard, gnu::always_inline]] static auto _expect(entry* const found) noexcept -> entry* {

				if (found == nullptr) [[unlikely]]
					ca::impl::key_not_found();
				return found;
			}

			template <typename Q, typename U>
			[[nodiscard]] auto _insert(const typename table::place spot, Q&& key, U&& value) noexcept -> ca::result<void> {

				auto made_key = ca::impl::make_from<K>(ca::forward<Q>(key));
				if (!made_key)
					return made_key.propagate();
				auto made_value = ca::impl::make_from<V>(ca::forward<U>(value));
				if (!made_value) {
					ca::error ignored{};
					ca::impl::clean(*made_key, ignored);
					return made_value.propagate();
				}
				ca::construct_at(_table.slot(spot._index), ca::move(*made_key), ca::move(*made_value));
				_table.commit(spot);
				return ca::result<void>{};
			}

			auto _erase(entry* const found) noexcept -> void {

				ca::destroy_at(found);
				_table.vacate(_table.index_of(found));
			}


			// -- assertions -------------------------------------------------

			static_assert(ca::is_object<K> && !ca::is_const<K> && !ca::is_array<K> && ca::is_object<V> && !ca::is_const<V> && !ca::is_array<V>,
				"ca::hashmap<K, V>: K and V must be non-const, non-array object types");

			static_assert(ca::is_hashable<ca::impl::lookup_of<K>>, "ca::hashmap<K, V>: K must be ca::hash-able");

	}; // class hashmap

} // namespace ca

#endif // ca_collections_map_hashmap_hpp
