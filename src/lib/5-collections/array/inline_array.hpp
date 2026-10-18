/* @DOC Performance of ca::inline_array<T, N>.

   Layout: the size (a usz), then room for N elements, in the object itself: no pointer, no
   allocation, no indirection. inline_array<int, 16> is 72 bytes, inline_array<u64, 4> 40. Slots
   past size() are raw storage: nothing is constructed there. The size is a full word on
   purpose: a u8 size made append/pop churn 2x slower with clang (partial-register updates in
   the loop-carried chain), for 7 bytes saved.

   | Operation | Time | Notes |
   |---|---|---|
   | operator[], get, first, last | O(1) | one compare against size() |
   | size, empty, full, begin, end, data, view | O(1) | capacity() is a constant expression |
   | append, emplace | O(1) | one compare against N, no growth path |
   | extend(view) | O(n) | one memcpy for trivially copyable T |
   | insert(i), pop(i) | O(size - i) | one memmove for trivially relocatable T |
   | swap_pop(i), pop() | O(1) | |
   | remove(value) | O(size) | find, then pop(i) |
   | resize, clear, destroy | O(abs(n - size)) | O(1) for trivially destructible T without destroy() |
   | clone, from(view) | O(n) | one memcpy for trivially copyable T |
   | find, count, contains, compare, == | O(n) | memchr/memcmp for code units, element loops otherwise |

   Nothing grows: what does not fit fails with errc::no_space and changes nothing. That is the
   point of it: bounded memory known at compile time, never a failed allocation, a place on the
   stack or inside another object for short lists of non-trivial elements (a function's
   arguments, a node's children, the last N events).

   Locality and alignment: the elements follow the size in one block aligned to alignof(T);
   reading one reaches the same cache lines as the object itself.

   Copy and move: trivially copyable when T is (copied as bytes, all N slots); otherwise copy is
   deleted, clone() is the explicit copy, and moving relocates the live elements and leaves the
   source empty. TRIVIALLY_RELOCATABLE when T is.

   Against std::vector with reserved capacity, at most N elements: no heap round trip at
   construction and destruction, append is one compare and one store. C++26 std::inplace_vector
   is the same structure.

   Baseline std::vector<int> over glibc malloc; ca is inline_array<int, 8>:
   --BENCH bench/small_arrays.cpp inline_array
   bench/small_arrays.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | 1e6 times build 4 elements and drop | 0.031x | 0.053x | |
   | 1e6 times build 8 elements and drop | 0.041x | 0.068x | |
   | churn, 1e7 appends, popping all at 8 | 0.38x | 1.33x | gcc: vectorized pops stall on store forwarding |
   BENCH--
   On the churn, gcc vectorizes the pops into 16-byte loads of slots stored one int at a time,
   a store-forwarding stall, and keeps the size in memory; it runs fewer instructions than
   std. */

#ifndef ca_collections_array_inline_array_hpp
#define ca_collections_array_inline_array_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/copy_construct_n.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/memory/destroy_n.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/memory/relocate_n.hpp"
#include "1-primitives/memory/relocate_overlapping_n.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_move_constructible.hpp"
#include "1-primitives/traits/is_object.hpp"
#include "1-primitives/traits/is_trivially_copyable.hpp"
#include "1-primitives/traits/is_trivially_destructible.hpp"
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
#include "3-system/error/errc.hpp"
#include "4-lifecycle/is_cleanable.hpp"
#include "4-lifecycle/is_clonable.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- INLINE ARRAY -------------------------------------------------------

	/* @DOC At most N elements of T stored in the object itself: a dynamic_array that never
	   allocates (C++26 std::inplace_vector, Rust's ArrayVec). Operations that would pass N fail
	   with errc::no_space and change nothing. Has the sequence methods (cut, find, compare, fill,
	   reverse) of ca::impl::slice_methods. Trivially copyable when T is; otherwise clone() copies
	   and moving leaves the source empty. Clean up with destroy() when T is cleanable; the
	   destructor is a failsafe. operator[] traps out of range; get allows the doubt. */
	template <typename T, ca::usz N>
	class inline_array final : public ca::impl::slice_methods<ca::inline_array<T, N>, T, const T> {


		private:

			// -- private types ----------------------------------------------

			using self = ca::inline_array<T, N>;


		public:

			// -- public types -----------------------------------------------

			using value_type      = T;
			using size_type       = ca::usz;
			using reference       = T&;
			using const_reference = const T&;
			using pointer         = T*;
			using const_pointer   = const T*;
			using iterator        = T*;
			using const_iterator  = const T*;


			// -- public constants -------------------------------------------

			/* @DOC Most elements held. */
			static constexpr ca::usz CAPACITY = N;

			/* @DOC No pointer into itself: containers move an inline_array with memcpy when T allows
			   it. */
			static constexpr bool TRIVIALLY_RELOCATABLE = ca::is_trivially_relocatable<T>;


		private:

			// -- private members --------------------------------------------

			ca::usz _size;
			union {
					T _slots[N];
			};


		public:

			// -- public lifecycle -------------------------------------------

			/* @DOC Empty; nothing is constructed. */
			constexpr inline_array(void) noexcept
			: _size{0U} {
			}

			inline_array(const self&) noexcept requires ca::is_trivially_copyable<T> = default;

			inline_array(const self&) requires (!ca::is_trivially_copyable<T>) = delete;

			inline_array(self&&) noexcept requires ca::is_trivially_copyable<T> = default;

			/* @DOC Relocates other's elements; other is left empty. */
			inline_array(self&& other) noexcept requires (!ca::is_trivially_copyable<T> && ca::is_move_constructible<T>)
			: _size{other._size} {

				ca::relocate_n(other._slots, other._size, _slots);
				other._size = 0U;
			}

			~inline_array(void) noexcept requires ca::is_trivially_destructible<T> = default;

			/* @DOC Failsafe for a skipped destroy(): destroys the elements, silently. */
			~inline_array(void) noexcept {
				ca::destroy_n(_slots, _size);
			}

			/* @DOC A copy of the elements of view: clone() of each when T has one, else the copy
			   constructor. errc::no_space when view holds more than N. On a failed element clone, the
			   partial copy is destroyed and that error returned. */
			[[nodiscard]] static auto from(const ca::borrowed_slice<const T> view) noexcept -> ca::result<self>
			requires ca::is_clonable<T> || ca::is_copy_constructible<T> {

				self copy{};
				const ca::usz count = view.size();
				if (count > N) [[unlikely]]
					return ca::failure{ca::errc::no_space};
				if constexpr (ca::is_clonable<T>) {
					for (const T& element : view) {
						auto cloned = element.clone();
						if (!cloned) {
							static_cast<void>(copy.destroy());
							return cloned.propagate();
						}
						ca::construct_at(copy._slots + copy._size, ca::move(*cloned));
						++copy._size;
					}
				}
				else {
					ca::copy_construct_n(view.data(), count, copy._slots);
					copy._size = count;
				}
				return copy;
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) noexcept -> self& requires ca::is_trivially_copyable<T> = default;

			auto operator=(const self&) -> self& requires (!ca::is_trivially_copyable<T>) = delete;

			auto operator=(self&&) noexcept -> self& requires ca::is_trivially_copyable<T> = default;

			/* @DOC Destroys the current elements as by the destructor, then relocates other's; other
			   is left empty. Traps when T is cleanable and elements remain: clear() or destroy()
			   first. */
			auto operator=(self&& other) noexcept -> self& requires (!ca::is_trivially_copyable<T> && ca::is_move_constructible<T>) {

				if (this == &other)
					return *this;
				if constexpr (ca::is_cleanable<T>) {
					if (_size != 0U) [[unlikely]]
						ca::impl::assigned_over_uncleaned_elements();
				}
				ca::destroy_n(_slots, _size);
				ca::relocate_n(other._slots, other._size, _slots);
				_size = ca::exchange(other._size, 0U);
				return *this;
			}


			// -- iterators --------------------------------------------------

			[[nodiscard]] constexpr auto begin(void) noexcept -> T* {
				return _slots;
			}

			[[nodiscard]] constexpr auto begin(void) const noexcept -> const T* {
				return _slots;
			}

			[[nodiscard]] constexpr auto end(void) noexcept -> T* {
				return _slots + _size;
			}

			[[nodiscard]] constexpr auto end(void) const noexcept -> const T* {
				return _slots + _size;
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
				return _size;
			}

			[[nodiscard]] static constexpr auto capacity(void) noexcept -> ca::usz {
				return N;
			}

			[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
				return _size == 0U;
			}

			[[nodiscard]] constexpr auto full(void) const noexcept -> bool {
				return _size == N;
			}

			[[nodiscard]] constexpr auto data(void) noexcept -> T* {
				return _slots;
			}

			[[nodiscard]] constexpr auto data(void) const noexcept -> const T* {
				return _slots;
			}


			// -- element access ---------------------------------------------

			/* @DOC Element at index. Traps when index >= size(), in every build. */
			[[nodiscard]] constexpr auto operator[](const ca::usz index) noexcept -> T& {

				this->_expect_index(index);
				return _slots[index];
			}

			[[nodiscard]] constexpr auto operator[](const ca::usz index) const noexcept -> const T& {

				this->_expect_index(index);
				return _slots[index];
			}

			/* @DOC Element at index, or nothing when index >= size(). */
			[[nodiscard]] constexpr auto get(const ca::usz index) noexcept -> ca::optional<T&> {

				if (index >= _size)
					return ca::nullopt;
				return ca::optional<T&>{_slots[index]};
			}

			[[nodiscard]] constexpr auto get(const ca::usz index) const noexcept -> ca::optional<const T&> {

				if (index >= _size)
					return ca::nullopt;
				return ca::optional<const T&>{_slots[index]};
			}


			// -- modifiers --------------------------------------------------

			/* @DOC Appends a T built from args; errc::no_space when full. */
			template <typename... Args>
			requires ca::is_constructible<T, Args...>
			[[nodiscard]] auto emplace(Args&&... args) noexcept -> ca::result<void> {

				if (_size == N) [[unlikely]]
					return ca::failure{ca::errc::no_space};
				ca::construct_at(_slots + _size, ca::forward<Args>(args)...);
				++_size;
				return ca::result<void>{};
			}

			/* @DOC emplace(value). */
			[[nodiscard]] auto append(const T& value) noexcept -> ca::result<void> requires ca::is_copy_constructible<T> {
				return this->emplace(value);
			}

			[[nodiscard]] auto append(T&& value) noexcept -> ca::result<void> requires ca::is_move_constructible<T> {
				return this->emplace(ca::move(value));
			}

			/* @DOC Copies the elements of view to the end, all or none (errc::no_space). view must not
			   be into this array. */
			[[nodiscard]] auto extend(const ca::borrowed_slice<const T> view) noexcept -> ca::result<void> requires ca::is_copy_constructible<T> {

				const ca::usz count = view.size();
				if (count > N - _size) [[unlikely]]
					return ca::failure{ca::errc::no_space};
				ca::copy_construct_n(view.data(), count, _slots + _size);
				_size += count;
				return ca::result<void>{};
			}

			/* @DOC Builds a T from args at index, shifting the elements after it; errc::no_space when
			   full. args may refer to an element. Traps when index > size(), in every build. */
			template <typename... Args>
			requires ca::is_constructible<T, Args...> && ca::is_move_constructible<T>
			[[nodiscard]] auto insert(const ca::usz index, Args&&... args) noexcept -> ca::result<void> {

				const ca::usz size = _size;
				if (index > size) [[unlikely]]
					ca::impl::index_out_of_bounds();
				if (size == N) [[unlikely]]
					return ca::failure{ca::errc::no_space};
				if (index == size)
					return this->emplace(ca::forward<Args>(args)...);
				T value(ca::forward<Args>(args)...);
				ca::relocate_overlapping_n(_slots + index, size - index, _slots + index + 1U);
				ca::construct_at(_slots + index, ca::move(value));
				++_size;
				return ca::result<void>{};
			}

			/* @DOC Copies the elements of view in at index, shifting the elements after it, all or
			   none (errc::no_space). view must not be into this array. Traps when index > size(), in
			   every build. */
			[[nodiscard]] auto insert(const ca::usz index, const ca::borrowed_slice<const T> view) noexcept -> ca::result<void>
			requires ca::is_copy_constructible<T> && ca::is_move_constructible<T> {

				const ca::usz size = _size;
				if (index > size) [[unlikely]]
					ca::impl::index_out_of_bounds();
				const ca::usz count = view.size();
				if (count > N - size) [[unlikely]]
					return ca::failure{ca::errc::no_space};
				ca::relocate_overlapping_n(_slots + index, size - index, _slots + index + count);
				ca::copy_construct_n(view.data(), count, _slots + index);
				_size = size + count;
				return ca::result<void>{};
			}

			/* @DOC Removes and returns the last element, or nothing when empty. */
			[[nodiscard]] auto pop(void) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {

				if (_size == 0U)
					return ca::nullopt;
				--_size;
				ca::optional<T> last{ca::in_place, ca::move(_slots[_size])};
				ca::destroy_at(_slots + _size);
				return last;
			}

			/* @DOC Removes and returns the element at index, shifting the elements after it, or
			   nothing when index >= size(). */
			[[nodiscard]] auto pop(const ca::usz index) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {

				const ca::usz size = _size;
				if (index >= size)
					return ca::nullopt;
				ca::optional<T> removed{ca::in_place, ca::move(_slots[index])};
				ca::destroy_at(_slots + index);
				ca::relocate_overlapping_n(_slots + index + 1U, size - index - 1U, _slots + index);
				--_size;
				return removed;
			}

			/* @DOC Removes and returns the first element equal to value, shifting the elements after
			   it, or nothing when none is. */
			[[nodiscard]] auto remove(const T& value) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {

				const ca::optional<ca::usz> found = this->find(value);
				if (!found)
					return ca::nullopt;
				return this->pop(*found);
			}

			/* @DOC Removes and returns the element at index in O(1), the last element taking its
			   place, or nothing when index >= size(). Does not keep order. */
			[[nodiscard]] auto swap_pop(const ca::usz index) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {

				if (index >= _size)
					return ca::nullopt;
				ca::optional<T> removed{ca::in_place, ca::move(_slots[index])};
				ca::destroy_at(_slots + index);
				--_size;
				if (index != _size)
					ca::relocate_n(_slots + _size, 1U, _slots + index);
				return removed;
			}

			/* @DOC Sets size() to count. Growing builds each new element from args (value-initialized
			   without), or changes nothing past N (errc::no_space). Shrinking cleans up and destroys
			   the tail; returns the first cleanup error, the tail is gone regardless. */
			template <typename... Args>
			requires ca::is_constructible<T, const Args&...>
			[[nodiscard]] auto resize(const ca::usz count, const Args&... args) noexcept -> ca::result<void> {

				const ca::usz size = _size;
				if (count <= size)
					return this->_truncate(count);
				if (count > N) [[unlikely]]
					return ca::failure{ca::errc::no_space};
				for (ca::usz i = size; i < count; ++i)
					ca::construct_at(_slots + i, args...);
				_size = count;
				return ca::result<void>{};
			}

			/* @DOC Cleans up and destroys every element. Returns the first cleanup error; every
			   element is gone regardless. */
			[[nodiscard]] auto clear(void) noexcept -> ca::result<void> {
				return this->_truncate(0U);
			}

			/* @DOC clear(): an inline_array owns no memory beyond itself. */
			[[nodiscard]] auto destroy(void) noexcept -> ca::result<void> {
				return this->_truncate(0U);
			}


			// -- methods ----------------------------------------------------

			/* @DOC The elements, as a view: what the shared methods work on. */
			[[nodiscard]] constexpr auto view(void) noexcept -> ca::borrowed_slice<T> {
				return ca::borrowed_slice<T>{_slots, _size};
			}

			[[nodiscard]] constexpr auto view(void) const noexcept -> ca::borrowed_slice<const T> {
				return ca::borrowed_slice<const T>{_slots, _size};
			}

			/* @DOC A copy; see from. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self> requires ca::is_clonable<T> || ca::is_copy_constructible<T> {
				return self::from(this->view());
			}


		private:

			// -- private methods --------------------------------------------

			constexpr auto _expect_index(const ca::usz index) const noexcept -> void {
				if (index >= _size) [[unlikely]]
					ca::impl::index_out_of_bounds();
			}

			[[nodiscard]] auto _truncate(const ca::usz count) noexcept -> ca::result<void> {

				T* const tail = _slots + count;
				T* const end = _slots + _size;
				_size = count;
				if constexpr (ca::is_cleanable<T>) {
					ca::error first{};
					for (T* element = tail; element != end; ++element) {
						element->destroy().match([] {
						}, [&first](const ca::error& error) {
							if (first.ok())
								first = error;
						});
						ca::destroy_at(element);
					}
					if (!first.ok())
						return ca::failure{first};
				}
				else
					ca::destroy_n(tail, static_cast<ca::usz>(end - tail));
				return ca::result<void>{};
			}


			// -- assertions -------------------------------------------------

			static_assert(ca::is_object<T> && !ca::is_const<T> && !ca::is_array<T>,
				"ca::inline_array<T, N>: T must be a non-const, non-array object type");

			static_assert(N != 0U, "ca::inline_array<T, N>: N must not be 0");

	}; // class inline_array

} // namespace ca

#endif // ca_collections_array_inline_array_hpp
