/* @DOC Performance of ca::intrusive_list<T, Tag>.

   Layout: first and last link pointers and the size, 24 bytes on 64-bit. The links live in the
   elements: each T derives from list_link<T, Tag>, two pointers, so the list allocates nothing
   and no operation can fail. Unlike ca::list, the elements are wherever their owner put them
   (an array, an arena, the stack, inside another object); the list only orders them.

   | Operation | Time | Notes |
   |---|---|---|
   | append, prepend, insert(position) | O(1) | four pointer stores, one linked() check |
   | remove(element), pop, pop_front | O(1) | no search: the element knows its neighbours |
   | iterator_to(element) | O(1) | |
   | splice(position, other) | O(1) | |
   | first, last, size, empty, begin, end | O(1) | |
   | reverse, clear | O(n) | clear unlinks every element |
   | sort | O(n log n) | stable merge on the links, as ca::list |

   Space: 16 bytes per element per list it can join (one list_link per Tag), nothing else.

   Locality: as ca::list, a dependent load per element; when the elements sit in an array, a
   walk in array order touches memory in order.

   Where it earns its place: free lists, LRU caches, run queues, timers and any object that
   must be found and unlinked in O(1) from a pointer to it, or be in several lists at once,
   without an allocation that could fail.

   Rules, checked: an element is in at most one list per Tag (linking a linked element traps);
   remove() takes elements of this list (removing an unlinked one traps); an element must leave
   its list before it is destroyed (its link's destructor traps otherwise). The list's
   destructor unlinks what it still holds. Copying or moving an element gives an unlinked one;
   relocating a linked element by bytes would corrupt the list, so list_link is not trivially
   copyable.

   Copy and move: the list is move-only; moving takes three words, the elements stay where they
   are. TRIVIALLY_RELOCATABLE: no element points at the list object. */

#ifndef ca_collections_list_intrusive_list_hpp
#define ca_collections_list_intrusive_list_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cv.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "1-primitives/utility/swap.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "5-collections/list/impl/merge_sort_links.hpp"
#include "5-collections/list/list_link.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- INTRUSIVE LIST -----------------------------------------------------

	template <typename T, typename Tag>
	class intrusive_list;


	namespace impl {

		/* @DOC Walks an intrusive_list both ways; end() steps back to the list's current last
		   element. An iterator converts to its const counterpart. */
		template <typename U, typename Tag>
		class intrusive_list_iterator final {


			private:

				// -- friends ------------------------------------------------

				template <typename, typename>
				friend class ca::intrusive_list;

				template <typename, typename>
				friend class ca::impl::intrusive_list_iterator;


				// -- private types ------------------------------------------

				using self = ca::impl::intrusive_list_iterator<U, Tag>;

				using link = ca::list_link<ca::remove_cv<U>, Tag>;


			public:

				// -- public types -------------------------------------------

				using value_type = ca::remove_cv<U>;
				using reference  = U&;
				using pointer    = U*;


			private:

				// -- private members ----------------------------------------

				link*        _link;
				link* const* _last;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr intrusive_list_iterator(link* const at, link* const* const last) noexcept
				: _link{at}, _last{last} {
				}

				/* @DOC The same position, read-only. */
				template <typename V>
				requires ca::is_const<U> && ca::is_same<const V, U>
				constexpr intrusive_list_iterator(const ca::impl::intrusive_list_iterator<V, Tag>& other) noexcept
				: _link{other._link}, _last{other._last} {
				}


				// -- element access -----------------------------------------

				/* @DOC The element. Traps at end(), in every build. */
				[[nodiscard]] constexpr auto operator*(void) const noexcept -> U& {

					if (_link == nullptr) [[unlikely]]
						ca::impl::index_out_of_bounds();
					return static_cast<U&>(*_link);
				}

				[[nodiscard]] constexpr auto operator->(void) const noexcept -> U* {
					return &**this;
				}


				// -- methods ------------------------------------------------

				/* @DOC The next position. Traps at end(), in every build. */
				constexpr auto operator++(void) noexcept -> self& {

					if (_link == nullptr) [[unlikely]]
						ca::impl::index_out_of_bounds();
					_link = _link->_next;
					return *this;
				}

				/* @DOC The previous position; from end(), the last element. Traps at the first
				   element, in every build. */
				constexpr auto operator--(void) noexcept -> self& {

					link* const previous = _link == nullptr ? *_last : _link->_previous;
					if (previous == nullptr) [[unlikely]]
						ca::impl::index_out_of_bounds();
					_link = previous;
					return *this;
				}

				[[nodiscard]] constexpr auto operator==(const self& other) const noexcept -> bool {
					return _link == other._link;
				}

		}; // class intrusive_list_iterator

	} // namespace impl


	/* @DOC Doubly linked list of elements it does not own, linked through the list_link<T, Tag>
	   they derive from: no allocation, nothing can fail, an element unlinks in O(1) from a
	   reference to it. Elements must stay in place while linked and leave before they die (both
	   checked by traps). first, last, pop and pop_front allow the doubt; dereferencing end()
	   traps. The destructor unlinks the remaining elements. */
	template <typename T, typename Tag = void>
	class intrusive_list final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::intrusive_list<T, Tag>;

			using link = ca::list_link<T, Tag>;


		public:

			// -- public types -----------------------------------------------

			using value_type      = T;
			using size_type       = ca::usz;
			using reference       = T&;
			using const_reference = const T&;
			using iterator        = ca::impl::intrusive_list_iterator<T, Tag>;
			using const_iterator  = ca::impl::intrusive_list_iterator<const T, Tag>;


			// -- public constants -------------------------------------------

			/* @DOC Two link pointers and the size; no element points at the list object. */
			static constexpr bool TRIVIALLY_RELOCATABLE = true;


		private:

			// -- private members --------------------------------------------

			link*   _first;
			link*   _last;
			ca::usz _size;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr intrusive_list(void) noexcept
			: _first{nullptr}, _last{nullptr}, _size{0U} {
			}

			intrusive_list(const self&) = delete;

			constexpr intrusive_list(self&& other) noexcept
			: _first{ca::exchange(other._first, nullptr)}, _last{ca::exchange(other._last, nullptr)}, _size{ca::exchange(other._size, 0U)} {
			}

			/* @DOC Unlinks the remaining elements. */
			constexpr ~intrusive_list(void) noexcept {
				this->clear();
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Unlinks the current elements, then takes other's. */
			constexpr auto operator=(self&& other) noexcept -> self& {

				if (this == &other)
					return *this;
				this->clear();
				_first = ca::exchange(other._first, nullptr);
				 _last = ca::exchange(other._last, nullptr);
				 _size = ca::exchange(other._size, 0U);
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

			/* @DOC Position of element, which must be in this list. Traps when it is in none. */
			[[nodiscard]] constexpr auto iterator_to(T& element) noexcept -> iterator {

				link& at = static_cast<link&>(element);
				if (!at.linked()) [[unlikely]]
					ca::impl::element_not_linked();
				return iterator{&at, &_last};
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
				return _size;
			}

			[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
				return _size == 0U;
			}


			// -- element access ---------------------------------------------

			/* @DOC First element, or nothing when empty. */
			[[nodiscard]] constexpr auto first(void) const noexcept -> ca::optional<T&> {

				if (_first == nullptr)
					return ca::nullopt;
				return ca::optional<T&>{static_cast<T&>(*_first)};
			}

			/* @DOC Last element, or nothing when empty. */
			[[nodiscard]] constexpr auto last(void) const noexcept -> ca::optional<T&> {

				if (_last == nullptr)
					return ca::nullopt;
				return ca::optional<T&>{static_cast<T&>(*_last)};
			}


			// -- modifiers --------------------------------------------------

			/* @DOC Links element at the end. Traps when it is already in a list through this Tag. */
			constexpr auto append(T& element) noexcept -> void {
				this->_link(nullptr, self::_free_link(element));
			}

			/* @DOC Links element at the front. Traps when it is already in a list through this Tag. */
			constexpr auto prepend(T& element) noexcept -> void {
				this->_link(_first, self::_free_link(element));
			}

			/* @DOC Links element before position (end() appends) and returns its position. Traps when
			   it is already in a list through this Tag. */
			constexpr auto insert(const const_iterator position, T& element) noexcept -> iterator {

				link* const at = self::_free_link(element);
				this->_link(position._link, at);
				return iterator{at, &_last};
			}

			/* @DOC Moves every element of other before position (end() appends), in O(1); other is
			   left empty. */
			constexpr auto splice(const const_iterator position, self&& other) noexcept -> void {

				if (this == &other || other._first == nullptr)
					return;
				link* const before = position._link;
				link* const previous = before == nullptr ? _last : before->_previous;
				other._first->_previous = previous;
				     other._last->_next = before;
				if (previous == nullptr)
					_first = other._first;
				else
					previous->_next = other._first;
				if (before == nullptr)
					_last = other._last;
				else
					before->_previous = other._last;
				       _size += other._size;
				other._first = nullptr;
				 other._last = nullptr;
				 other._size = 0U;
			}

			/* @DOC Unlinks element, which must be in this list. Traps when it is in none. */
			constexpr auto remove(T& element) noexcept -> void {

				link& at = static_cast<link&>(element);
				if (!at.linked()) [[unlikely]]
					ca::impl::element_not_linked();
				this->_unlink(&at);
			}

			/* @DOC Unlinks and returns the last element, or nothing when empty. */
			[[nodiscard]] constexpr auto pop(void) noexcept -> ca::optional<T&> {
				return this->_take(_last);
			}

			/* @DOC Unlinks and returns the first element, or nothing when empty. */
			[[nodiscard]] constexpr auto pop_front(void) noexcept -> ca::optional<T&> {
				return this->_take(_first);
			}

			/* @DOC Reverses the order by swapping links. */
			constexpr auto reverse(void) noexcept -> void {

				for (link* at = _first; at != nullptr; at = at->_previous)
					ca::swap(at->_next, at->_previous);
				ca::swap(_first, _last);
			}

			/* @DOC Sorts by less(a, b), stable, in O(n log n) by merging runs of links. */
			template <typename F>
			constexpr auto sort(F&& less) noexcept -> void {

				if (_size < 2U)
					return;
				const ca::impl::link_run<link> sorted = ca::impl::merge_sort_links(_first, [&less](const link* const left, const link* const right) {
					return less(static_cast<const T&>(*left), static_cast<const T&>(*right));
				});
				_first = sorted._first;
				 _last = sorted._last;
			}

			/* @DOC sort by <. */
			constexpr auto sort(void) noexcept -> void {
				this->sort([](const T& left, const T& right) {
					return left < right;
				});
			}

			/* @DOC Unlinks every element; they are left as they were otherwise. */
			constexpr auto clear(void) noexcept -> void {

				for (link* at = _first; at != nullptr;) {
					link* const next = at->_next;
					self::_detach(at);
					at = next;
				}
				_first = nullptr;
				 _last = nullptr;
				 _size = 0U;
			}


		private:

			// -- private methods --------------------------------------------

			[[nodiscard]] static constexpr auto _free_link(T& element) noexcept -> link* {

				link& at = static_cast<link&>(element);
				if (at.linked()) [[unlikely]]
					ca::impl::element_already_linked();
				return &at;
			}

			constexpr auto _link(link* const before, link* const at) noexcept -> void {

				link* const previous = before == nullptr ? _last : before->_previous;
				at->_previous = previous;
				    at->_next = before;
				if (previous == nullptr)
					_first = at;
				else
					previous->_next = at;
				if (before == nullptr)
					_last = at;
				else
					before->_previous = at;
				++_size;
			}

			constexpr auto _unlink(link* const at) noexcept -> void {

				if (at->_previous == nullptr)
					_first = at->_next;
				else
					at->_previous->_next = at->_next;
				if (at->_next == nullptr)
					_last = at->_previous;
				else
					at->_next->_previous = at->_previous;
				self::_detach(at);
				--_size;
			}

			static constexpr auto _detach(link* const at) noexcept -> void {

				at->_previous = nullptr;
				    at->_next = at;
			}

			[[nodiscard]] constexpr auto _take(link* const at) noexcept -> ca::optional<T&> {

				if (at == nullptr)
					return ca::nullopt;
				this->_unlink(at);
				return ca::optional<T&>{static_cast<T&>(*at)};
			}

	}; // class intrusive_list

} // namespace ca

#endif // ca_collections_list_intrusive_list_hpp
