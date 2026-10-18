/* @DOC Performance of ca::list<T, A>.

   Layout: first and last node pointers and the size, 24 bytes on 64-bit, plus the allocator:
   nothing for a stateless one (ca::heap, the default). Each element lives in its own node,
   two links then the element, in one block from A: sizeof(T) + 16 rounded up to alignof(T) and
   to the allocator's size class (32 bytes for an int on the heap). No sentinel node: an empty
   list owns no memory, and nothing points back into the list object.

   | Operation | Time | Notes |
   |---|---|---|
   | append, prepend, emplace, emplace_front, insert(position) | O(1) | one allocation |
   | pop, pop_front, pop(position) | O(1) | one deallocation |
   | first, last, size, empty, begin, end | O(1) | |
   | splice(position, other) | O(1) | no allocation, no element touched |
   | extend(view) | O(n) | n allocations, linked once all succeed |
   | find, count, contains, remove(value) | O(n) | pointer chasing |
   | retain(test) | O(n) | |
   | reverse | O(n) | swaps the links, elements stay in place |
   | sort | O(n log n) | stable merge sort on the links: no allocation, elements never move |
   | clear, destroy, clone | O(n) | |

   Space: 16 bytes of links per element plus the allocator's rounding; an arena allocator makes
   nodes as dense as an array of nodes.

   Locality: nodes go wherever the allocator puts them; walking the list is a dependent load per
   element, several times slower than an array scan once the list leaves the cache. Prefer
   dynamic_array or deque for sequences that are mostly scanned or indexed. A list earns its
   place where elements must not move (pointers to them stay valid for their lifetime: no
   relocation ever), where splicing whole runs in O(1) matters, or where elements are inserted
   and removed in the middle through positions already held.

   Copy and move: the copy constructor is deleted; clone() is the explicit, fallible copy. Move
   construction and assignment take three words and the allocator, no allocation; iterators keep
   pointing at the same elements but end() and stepping back from it follow the list object,
   so they belong to the new owner. TRIVIALLY_RELOCATABLE when A is.

   Inlining: append, prepend and pop are the allocator's fast path plus four pointer stores.

   Sort merges as a binary counter of pending runs (libstdc++'s scheme): each node is merged
   while still in cache. Bottom-up passes over the whole list were 1.5x slower than std on 1M
   scattered nodes.

   Against std::list (sentinel node in the object, so not relocatable): same node shape and
   complexities. Baseline std::list over glibc malloc; ca over ca::heap:
   --BENCH bench/list.cpp
   bench/list.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | append 1e6, then destroy | 0.33x | 0.36x | |
   | prepend twice, pop_front once, 1e6 times | 0.53x | 0.29x | |
   | walk 1e6 nodes 10 times | 0.54x | 0.60x | the heap packs nodes of one size class together |
   | sort 1e6 scrambled ints | 0.99x | 0.84x | |
   | insert + pop at a held middle position, 1e7 | 0.34x | 0.35x | |
   | build 1e4 runs of 100 and splice each | 0.058x | 0.075x | splice is O(1) in both; the gap is node allocation |
   BENCH-- */

#ifndef ca_collections_list_list_hpp
#define ca_collections_list_list_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_move_constructible.hpp"
#include "1-primitives/traits/is_object.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cv.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "1-primitives/utility/forward.hpp"
#include "1-primitives/utility/move.hpp"
#include "1-primitives/utility/swap.hpp"
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
#include "5-collections/list/impl/merge_sort_links.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- LIST ---------------------------------------------------------------

	template <typename T, ca::is_allocator A>
	class list;


	namespace impl {

		/* @DOC One element of a list and its two links, in one block. */
		template <typename T>
		struct list_node final {

				using self = ca::impl::list_node<T>;

				self* _previous;
				self* _next;
				T     _value;

				template <typename... Args>
				constexpr explicit list_node(ca::in_place_t, Args&&... args) noexcept
				: _previous{nullptr}, _next{nullptr}, _value(ca::forward<Args>(args)...) { // init-list-brace: ignore
				}

		}; // struct list_node


		/* @DOC Walks a list both ways. end() is the position past the last element; stepping back
		   from it reads the list's current last node, so it follows the list object. An iterator
		   converts to its const counterpart. */
		template <typename T>
		class list_iterator final {


			private:

				// -- friends ------------------------------------------------

				template <typename, ca::is_allocator>
				friend class ca::list;

				template <typename>
				friend class ca::impl::list_iterator;


				// -- private types ------------------------------------------

				using self = ca::impl::list_iterator<T>;

				using node = ca::impl::list_node<ca::remove_cv<T>>;


			public:

				// -- public types -------------------------------------------

				using value_type = ca::remove_cv<T>;
				using reference  = T&;
				using pointer    = T*;


			private:

				// -- private members ----------------------------------------

				node*              _node;
				node* const*       _last;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr list_iterator(node* const at, node* const* const last) noexcept
				: _node{at}, _last{last} {
				}

				/* @DOC The same position, read-only. */
				template <typename U>
				requires ca::is_const<T> && ca::is_same<const U, T>
				constexpr list_iterator(const ca::impl::list_iterator<U>& other) noexcept
				: _node{other._node}, _last{other._last} {
				}


				// -- element access -----------------------------------------

				/* @DOC The element. Traps at end(), in every build. */
				[[nodiscard]] constexpr auto operator*(void) const noexcept -> T& {

					if (_node == nullptr) [[unlikely]]
						ca::impl::index_out_of_bounds();
					return _node->_value;
				}

				[[nodiscard]] constexpr auto operator->(void) const noexcept -> T* {

					if (_node == nullptr) [[unlikely]]
						ca::impl::index_out_of_bounds();
					return &_node->_value;
				}


				// -- methods ------------------------------------------------

				/* @DOC The next position. Traps at end(), in every build. */
				constexpr auto operator++(void) noexcept -> self& {

					if (_node == nullptr) [[unlikely]]
						ca::impl::index_out_of_bounds();
					_node = _node->_next;
					return *this;
				}

				/* @DOC The previous position; from end(), the last element. Traps at the first
				   element, in every build. */
				constexpr auto operator--(void) noexcept -> self& {

					node* const previous = _node == nullptr ? *_last : _node->_previous;
					if (previous == nullptr) [[unlikely]]
						ca::impl::index_out_of_bounds();
					_node = previous;
					return *this;
				}

				[[nodiscard]] constexpr auto operator==(const self& other) const noexcept -> bool {
					return _node == other._node;
				}

		}; // class list_iterator

	} // namespace impl


	/* @DOC Doubly linked list of T, one node per element in memory from A. Elements never move:
	   pointers and iterators to one stay valid until it is removed. append, prepend and insert
	   at a held position in O(1), each acquiring a node and failing with nothing changed;
	   splice moves another list's nodes in O(1); sort is a stable merge on the links.
	   Construction allocates nothing. Clean up with destroy(); the destructor is a failsafe that
	   frees silently. first and last allow the doubt; dereferencing end() traps. A stateful A has
	   no default construction and moves with the elements. */
	template <typename T, ca::is_allocator A = ca::heap>
	class list final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::list<T, A>;

			using node = ca::impl::list_node<T>;


		public:

			// -- public types -----------------------------------------------

			using value_type      = T;
			using allocator_type  = A;
			using size_type       = ca::usz;
			using reference       = T&;
			using const_reference = const T&;
			using iterator        = ca::impl::list_iterator<T>;
			using const_iterator  = ca::impl::list_iterator<const T>;

			/* @DOC The same list type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::list<T, B>;


			// -- public constants -------------------------------------------

			/* @DOC Two node pointers, the size and the allocator; no node points at the list object:
			   containers move a list with memcpy when A allows it. */
			static constexpr bool TRIVIALLY_RELOCATABLE = ca::is_trivially_relocatable<A>;


		private:

			// -- private members --------------------------------------------

			node*                   _first;
			node*                   _last;
			ca::usz                 _size;
			[[no_unique_address]] A _allocator;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr list(void) noexcept requires ca::is_stateless_allocator<A>
			: _first{nullptr}, _last{nullptr}, _size{0U}, _allocator{} {
			}

			/* @DOC Empty, drawing nodes from allocator. */
			constexpr explicit list(const A& allocator) noexcept
			: _first{nullptr}, _last{nullptr}, _size{0U}, _allocator{allocator} {
			}

			list(const self&) = delete;

			constexpr list(self&& other) noexcept
			: _first{ca::exchange(other._first, nullptr)}, _last{ca::exchange(other._last, nullptr)}, _size{ca::exchange(other._size, 0U)},
			  _allocator{other._allocator} {
			}

			/* @DOC Failsafe for a skipped destroy(): destroys the elements and frees, silently. */
			~list(void) noexcept {
				this->_release();
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Takes other's nodes and allocator; the previous contents are released as by the
			   destructor. Traps when T is cleanable and elements remain: destroy() first. */
			auto operator=(self&& other) noexcept -> self& {

				if (this == &other)
					return *this;
				if constexpr (ca::is_cleanable<T>) {
					if (_size != 0U) [[unlikely]]
						ca::impl::assigned_over_uncleaned_elements();
				}
				this->_release();
				    _first = ca::exchange(other._first, nullptr);
				     _last = ca::exchange(other._last, nullptr);
				     _size = ca::exchange(other._size, 0U);
				_allocator = other._allocator;
				return *this;
			}


			// -- iterators --------------------------------------------------

			[[nodiscard]] constexpr auto begin(void) noexcept -> iterator {
				return iterator{_first, &_last};
			}

			[[nodiscard]] constexpr auto begin(void) const noexcept -> const_iterator {
				return const_iterator{_first, &_last};
			}

			[[nodiscard]] constexpr auto end(void) noexcept -> iterator {
				return iterator{nullptr, &_last};
			}

			[[nodiscard]] constexpr auto end(void) const noexcept -> const_iterator {
				return const_iterator{nullptr, &_last};
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

			/* @DOC First element, or nothing when empty. */
			[[nodiscard]] constexpr auto first(void) noexcept -> ca::optional<T&> {

				if (_first == nullptr)
					return ca::nullopt;
				return ca::optional<T&>{_first->_value};
			}

			[[nodiscard]] constexpr auto first(void) const noexcept -> ca::optional<const T&> {

				if (_first == nullptr)
					return ca::nullopt;
				return ca::optional<const T&>{_first->_value};
			}

			/* @DOC Last element, or nothing when empty. */
			[[nodiscard]] constexpr auto last(void) noexcept -> ca::optional<T&> {

				if (_last == nullptr)
					return ca::nullopt;
				return ca::optional<T&>{_last->_value};
			}

			[[nodiscard]] constexpr auto last(void) const noexcept -> ca::optional<const T&> {

				if (_last == nullptr)
					return ca::nullopt;
				return ca::optional<const T&>{_last->_value};
			}


			// -- modifiers --------------------------------------------------

			/* @DOC Appends a T built from args. On failure nothing changes. */
			template <typename... Args>
			requires ca::is_constructible<T, Args...>
			[[nodiscard]] auto emplace(Args&&... args) noexcept -> ca::result<void> {

				auto made = self::_new_node(_allocator, ca::forward<Args>(args)...);
				if (!made) [[unlikely]]
					return made.propagate();
				this->_link(nullptr, *made, *made, 1U);
				return ca::result<void>{};
			}

			/* @DOC Builds a T from args before the first element. On failure nothing changes. */
			template <typename... Args>
			requires ca::is_constructible<T, Args...>
			[[nodiscard]] auto emplace_front(Args&&... args) noexcept -> ca::result<void> {

				auto made = self::_new_node(_allocator, ca::forward<Args>(args)...);
				if (!made) [[unlikely]]
					return made.propagate();
				this->_link(_first, *made, *made, 1U);
				return ca::result<void>{};
			}

			/* @DOC emplace(value). */
			[[nodiscard]] auto append(const T& value) noexcept -> ca::result<void> requires ca::is_copy_constructible<T> {
				return this->emplace(value);
			}

			[[nodiscard]] auto append(T&& value) noexcept -> ca::result<void> requires ca::is_move_constructible<T> {
				return this->emplace(ca::move(value));
			}

			/* @DOC emplace_front(value). */
			[[nodiscard]] auto prepend(const T& value) noexcept -> ca::result<void> requires ca::is_copy_constructible<T> {
				return this->emplace_front(value);
			}

			[[nodiscard]] auto prepend(T&& value) noexcept -> ca::result<void> requires ca::is_move_constructible<T> {
				return this->emplace_front(ca::move(value));
			}

			/* @DOC Builds a T from args before position (end() appends) and returns its position. On
			   failure nothing changes. */
			template <typename... Args>
			requires ca::is_constructible<T, Args...>
			[[nodiscard]] auto insert(const const_iterator position, Args&&... args) noexcept -> ca::result<iterator> {

				auto made = self::_new_node(_allocator, ca::forward<Args>(args)...);
				if (!made) [[unlikely]]
					return made.propagate();
				this->_link(position._node, *made, *made, 1U);
				return iterator{*made, &_last};
			}

			/* @DOC Copies the elements of view to the end, all or none: the nodes are linked once
			   every one is made. */
			[[nodiscard]] auto extend(const ca::borrowed_slice<const T> view) noexcept -> ca::result<void> requires ca::is_copy_constructible<T> {

				if (view.empty())
					return ca::result<void>{};
				node* head = nullptr;
				node* tail = nullptr;
				for (const T& element : view) {
					auto made = self::_new_node(_allocator, element);
					if (!made) [[unlikely]] {
						self::_free_chain(_allocator, head);
						return made.propagate();
					}
					(*made)->_previous = tail;
					if (tail == nullptr)
						head = *made;
					else
						tail->_next = *made;
					tail = *made;
				}
				this->_link(nullptr, head, tail, view.size());
				return ca::result<void>{};
			}

			/* @DOC Moves every node of other before position (end() appends), in O(1); other is left
			   empty. Stateless allocators only, where any instance frees what another allocated. */
			auto splice(const const_iterator position, self&& other) noexcept -> void requires ca::is_stateless_allocator<A> {

				if (this == &other || other._first == nullptr)
					return;
				this->_link(position._node, other._first, other._last, other._size);
				other._first = nullptr;
				 other._last = nullptr;
				 other._size = 0U;
			}

			/* @DOC Removes and returns the last element, or nothing when empty. */
			[[nodiscard]] auto pop(void) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {
				return this->_take(_last);
			}

			/* @DOC Removes and returns the first element, or nothing when empty. */
			[[nodiscard]] auto pop_front(void) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {
				return this->_take(_first);
			}

			/* @DOC Removes and returns the element at position, or nothing at end(). Other positions
			   stay valid. */
			[[nodiscard]] auto pop(const const_iterator position) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {
				return this->_take(position._node);
			}

			/* @DOC Removes and returns the first element equal to value, or nothing when none is. */
			[[nodiscard]] auto remove(const T& value) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {

				for (node* at = _first; at != nullptr; at = at->_next) {
					if (at->_value == value)
						return this->_take(at);
				}
				return ca::nullopt;
			}

			/* @DOC Keeps the elements for which test(element) holds, in order; the others are cleaned
			   up and destroyed. Returns the first cleanup error; they are gone regardless. */
			template <typename F>
			[[nodiscard]] auto retain(F&& test) noexcept -> ca::result<void> {

				ca::error first{};
				for (node* at = _first; at != nullptr;) {
					node* const next = at->_next;
					if (!test(static_cast<const T&>(at->_value))) {
						this->_unlink(at);
						self::_clean(at, first);
						self::_free_node(_allocator, at);
					}
					at = next;
				}
				if (!first.ok())
					return ca::failure{first};
				return ca::result<void>{};
			}

			/* @DOC Reverses the order by swapping links; no element moves. */
			auto reverse(void) noexcept -> void {

				for (node* at = _first; at != nullptr; at = at->_previous)
					ca::swap(at->_next, at->_previous);
				node* const first = _first;
				_first = _last;
				 _last = first;
			}

			/* @DOC Sorts by less(a, b), stable, in O(n log n) by merging runs of links: no allocation,
			   no element moves. */
			template <typename F>
			auto sort(F&& less) noexcept -> void {

				if (_size < 2U)
					return;
				const ca::impl::link_run<node> sorted = ca::impl::merge_sort_links(_first, [&less](const node* const left, const node* const right) {
					return less(static_cast<const T&>(left->_value), static_cast<const T&>(right->_value));
				});
				_first = sorted._first;
				 _last = sorted._last;
			}

			/* @DOC sort by <. */
			auto sort(void) noexcept -> void {
				this->sort([](const T& left, const T& right) {
					return left < right;
				});
			}

			/* @DOC Cleans up, destroys and frees every element. Returns the first cleanup error;
			   every element is gone regardless. */
			[[nodiscard]] auto clear(void) noexcept -> ca::result<void> {

				ca::error first{};
				for (node* at = _first; at != nullptr;) {
					node* const next = at->_next;
					self::_clean(at, first);
					self::_free_node(_allocator, at);
					at = next;
				}
				_first = nullptr;
				 _last = nullptr;
				 _size = 0U;
				if (!first.ok())
					return ca::failure{first};
				return ca::result<void>{};
			}

			/* @DOC clear(): a list keeps no memory beyond its nodes. */
			[[nodiscard]] auto destroy(void) noexcept -> ca::result<void> {
				return this->clear();
			}


			// -- methods ----------------------------------------------------

			/* @DOC Position of the first element equal to value, or nothing. */
			[[nodiscard]] constexpr auto find(const T& value) noexcept -> ca::optional<iterator> {

				for (node* at = _first; at != nullptr; at = at->_next) {
					if (at->_value == value)
						return iterator{at, &_last};
				}
				return ca::nullopt;
			}

			[[nodiscard]] constexpr auto find(const T& value) const noexcept -> ca::optional<const_iterator> {

				for (node* at = _first; at != nullptr; at = at->_next) {
					if (at->_value == value)
						return const_iterator{at, &_last};
				}
				return ca::nullopt;
			}

			[[nodiscard]] constexpr auto contains(const T& value) const noexcept -> bool {
				return this->find(value).has_value();
			}

			/* @DOC Elements equal to value. */
			[[nodiscard]] constexpr auto count(const T& value) const noexcept -> ca::usz {

				ca::usz found = 0U;
				for (const node* at = _first; at != nullptr; at = at->_next)
					found += at->_value == value ? 1U : 0U;
				return found;
			}

			/* @DOC A copy with nodes from the same allocator. Elements are copied with clone() when T
			   has one, else with the copy constructor. On a failure, the partial copy is destroyed
			   and that error returned. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self> requires ca::is_clonable<T> || ca::is_copy_constructible<T> {
				return this->clone_in(_allocator);
			}

			/* @DOC clone, with nodes from allocator. */
			[[nodiscard]] auto clone_in(const A& allocator) const noexcept -> ca::result<self> requires ca::is_clonable<T> || ca::is_copy_constructible<T> {

				self copy{allocator};
				for (const node* at = _first; at != nullptr; at = at->_next) {
					if constexpr (ca::is_clonable<T>) {
						auto cloned = at->_value.clone();
						if (!cloned) {
							static_cast<void>(copy.destroy());
							return cloned.propagate();
						}
						auto added = copy.emplace(ca::move(*cloned));
						if (!added) {
							static_cast<void>(cloned->destroy());
							static_cast<void>(copy.destroy());
							return added.propagate();
						}
					}
					else {
						auto added = copy.emplace(at->_value);
						if (!added) {
							static_cast<void>(copy.destroy());
							return added.propagate();
						}
					}
				}
				return copy;
			}


		private:

			// -- private methods --------------------------------------------

			template <typename... Args>
			[[nodiscard, gnu::always_inline]] static auto _new_node(const A allocator, Args&&... args) noexcept -> ca::result<node*> {

				auto block = ca::allocate_n<node>(allocator, 1U);
				if (!block) [[unlikely]]
					return block.propagate();
				return ca::construct_at(static_cast<node*>(block->data()), ca::in_place, ca::forward<Args>(args)...);
			}

			[[gnu::always_inline]] static auto _free_node(const A allocator, node* const at) noexcept -> void {

				ca::destroy_at(at);
				ca::deallocate_n<node>(allocator, ca::allocation{at, sizeof(node)});
			}

			static auto _free_chain(const A allocator, node* at) noexcept -> void {
				while (at != nullptr) {
					node* const next = at->_next;
					self::_free_node(allocator, at);
					at = next;
				}
			}

			static auto _clean(node* const at, ca::error& first) noexcept -> void {
				if constexpr (ca::is_cleanable<T>) {
					at->_value.destroy().match([] {
					}, [&first](const ca::error& error) {
						if (first.ok())
							first = error;
					});
				}
			}

			auto _link(node* const before, node* const head, node* const tail, const ca::usz count) noexcept -> void {

				node* const previous = before == nullptr ? _last : before->_previous;
				head->_previous = previous;
				    tail->_next = before;
				if (previous == nullptr)
					_first = head;
				else
					previous->_next = head;
				if (before == nullptr)
					_last = tail;
				else
					before->_previous = tail;
				_size += count;
			}

			auto _unlink(node* const at) noexcept -> void {

				if (at->_previous == nullptr)
					_first = at->_next;
				else
					at->_previous->_next = at->_next;
				if (at->_next == nullptr)
					_last = at->_previous;
				else
					at->_next->_previous = at->_previous;
				--_size;
			}

			[[nodiscard]] auto _take(node* const at) noexcept -> ca::optional<T> {

				if (at == nullptr)
					return ca::nullopt;
				this->_unlink(at);
				ca::optional<T> taken{ca::in_place, ca::move(at->_value)};
				self::_free_node(_allocator, at);
				return taken;
			}

			auto _release(void) noexcept -> void {

				self::_free_chain(_allocator, _first);
				_first = nullptr;
				 _last = nullptr;
				 _size = 0U;
			}


			// -- assertions -------------------------------------------------

			static_assert(ca::is_object<T> && !ca::is_const<T> && !ca::is_array<T>,
				"ca::list<T>: T must be a non-const, non-array object type");

	}; // class list

} // namespace ca

#endif // ca_collections_list_list_hpp
