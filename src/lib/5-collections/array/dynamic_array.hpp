/* @DOC Performance of ca::dynamic_array<T, A>.

   Layout: three pointers (data, end, limit), 24 bytes on 64-bit, plus the allocator: nothing for
   a stateless one (ca::heap, the default), its size for a stateful one (8 for arena_allocator).
   Empty arrays own no memory.
   Pointers instead of size/capacity integers: under strict aliasing, storing an integer element
   cannot clobber a pointer member, so end stays in a register across pushes and index loops
   (char/byte elements still alias everything). size() and capacity() cost a subtraction and a
   shift, or a multiply for sizes that are not powers of two.

   | Operation | Time | Notes |
   |---|---|---|
   | operator[], get, first, last | O(1) | one compare; in `i < size()` loops it folds into the loop bound |
   | size, capacity, empty, begin, end, data | O(1) | |
   | append, emplace | amortized O(1) | O(n) on the growth step |
   | extend(view) | amortized O(n), one memcpy for trivial T | O(size + n) on the growth step |
   | pop() | O(1) | one emptiness compare (std::vector::pop_back has none) |
   | insert(i), pop(i) | O(n - i) | one memmove for trivially relocatable T |
   | insert(i, view) | O(n - i + m) | one memmove and one memcpy for trivial T |
   | remove(value) | O(n) | find, then pop(i) |
   | swap_pop(i) | O(1) | the last element relocates into the hole; order not kept |
   | reserve, shrink | O(n) | trivially relocatable T: ca::reallocate (heap: in place within a size class, page remap past 128 KiB, no copy); others try expand first |
   | resize(n) | O(abs(n - size)) | plus the growth step |
   | clear, cleanup | O(n) | O(1) for trivially destructible T without cleanup() |
   | clone | O(n) | one memcpy for trivially copyable T |
   | view, prefix, suffix, sub | O(1) | borrowed_slice: two words, no allocation |
   | find, count, contains, compare, == | O(n) | memchr/memcmp for code units, element loops otherwise |
   | into_slice | O(1) | the block changes owner, no copy |

   Space: capacity doubles, the first allocation holds 4 elements, and capacity is the
   allocator's usable size: the heap rounds to its size class (at most 25% over past 128 bytes),
   so up to 2.5x the live elements after growth. shrink() trims to the allocator's rounding. Up
   to 4 elements cost one allocation (std::vector grows 1, 2, 4).

   Locality and alignment: elements are contiguous in index order with stride sizeof(T), in a
   block aligned to alignof(T) (the heap gives 16 at least, more on request). Linear scans are
   prefetcher-friendly and vectorize, through iterators or indices. The header lives apart from
   the elements.

   Relocation: growth, shrink, insert and remove move elements by relocation. For trivially
   copyable T, and for types opting in with TRIVIALLY_RELOCATABLE (dynamic_array itself), that is
   memcpy/memmove, and reserve/shrink use ca::reallocate; an outer array never touches its inner
   arrays' blocks. Other types are move-constructed and destroyed one by one, in a single pass.

   Growth and aliasing: appending a T (or nothing) to a full array of trivially copyable T copies
   the value first, then reallocates; `a.append(a[0])` stays valid. Other arguments are built into
   the new block before the old one is released. Either way a failed growth changes nothing.

   Copy and move: the copy constructor is deleted; clone() is the explicit, fallible copy, in the
   same allocator (clone_in for another). Move construction and assignment take three pointers
   and the allocator, noexcept, no allocation. Not trivially
   copyable (destructor failsafe): passed and returned through memory, so take it by reference
   and return it by value (NRVO, result<dynamic_array>).

   Inlining: the fast paths (append, emplace, operator[], pop) are a handful of instructions.
   Growth runs in out-of-line static helpers that receive pointers, sizes and the allocator by
   value, never `this` nor a reference to the allocator member: the caller's array is not
   address-taken, its members stay in registers. Inlining the heap's fast path there instead
   cost 5x on append/pop churn with gcc.

   Cleanup: cleanup() and clear() run each element's cleanup() and destructor in one pass.

   Baseline std::vector (std::allocator over glibc malloc); ca over ca::heap (heap) and over
   glibc malloc through ca::libc_allocator (libc), which isolates the container from the
   allocator:
   --BENCH bench/dynamic_array.cpp
   bench/dynamic_array.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 heap | clang 18 libc | gcc 13 heap | gcc 13 libc | Notes |
   |---|---|---|---|---|---|
   | append 1e7 ints, growing | 0.30x | 0.30x | 0.41x | 0.41x | heap: page remap past 128 KiB, no copy; gcc's figure moves with loop placement |
   | append 1e7 u64, reserved | 0.81x | 0.81x | 1.00x | 1.00x | |
   | append 1e6 nested arrays | 0.51x | 1.24x | 0.45x | 1.19x | trivially relocatable: growth is a memcpy |
   | resize to 1e7 ints | 0.99x | 0.99x | 0.99x | 1.00x | |
   | index write, 1e7 u64 | 0.99x | 0.96x | 1.24x | 1.01x | |
   | index sum, 1e7 u64 | 0.99x | 1.01x | 0.99x | 0.99x | the bound check folds into the loop bound |
   | range-for sum, 1e7 u64 | 0.97x | 0.98x | 0.99x | 0.99x | |
   | index write, 1e7 ints | 0.97x | 0.93x | 0.99x | 1.01x | |
   | clone 1e7 ints | 0.99x | 0.97x | 1.00x | 0.99x | one memcpy |
   | clone 1e6 nested arrays | 0.44x | 1.01x | 0.49x | 1.02x | |
   | insert 2e4 ints at the front | 1.00x | 1.01x | 1.01x | 1.00x | |
   | insert 5e3 nested arrays at the front | 0.26x | 0.27x | 0.27x | 0.27x | relocation by memmove, no move constructor |
   | pop 2e4 ints from the front | 0.98x | 0.99x | 0.99x | 0.98x | |
   | append/pop churn, 1e5 ints x 100 | 1.29x | 1.33x | 1.13x | 1.13x | checked pop: one emptiness compare std skips |
   | 1e6 arrays of 3 ints | 0.34x | 0.63x | 0.19x | 0.36x | one heap block each |
   BENCH-- */

#ifndef ca_collections_array_dynamic_array_hpp
#define ca_collections_array_dynamic_array_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/limits.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/copy_construct_n.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/memory/destroy_n.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/memory/relocate_n.hpp"
#include "1-primitives/memory/relocate_overlapping_n.hpp"
#include "1-primitives/numeric/add_overflow.hpp"
#include "1-primitives/numeric/max.hpp"
#include "1-primitives/numeric/mul_overflow.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_move_constructible.hpp"
#include "1-primitives/traits/is_object.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/is_trivially_copyable.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
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
#include "4-lifecycle/memory/allocate_n.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/deallocate_n.hpp"
#include "4-lifecycle/memory/expand_n.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "4-lifecycle/memory/reallocate_n.hpp"
#include "5-collections/array/slice.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DYNAMIC ARRAY ------------------------------------------------------

	/* @DOC Contiguous, growable array of T in memory from A: Python's list. Construction allocates
	   nothing; reserve, append, extend, emplace, insert, resize, shrink and clone acquire memory
	   and can fail. Clean up with cleanup(); the destructor is a failsafe that frees silently.
	   operator[] traps out of range; get allows the doubt. The sequence methods (cut, find, compare, fill, reverse) come
	   from ca::impl::slice_methods and return borrowed_slices into it; into_slice fixes the
	   length. T may be incomplete until a member is used. A stateful A has no default
	   construction and moves with the elements. */
	template <typename T, ca::is_allocator A = ca::heap>
	class dynamic_array final : public ca::impl::slice_methods<ca::dynamic_array<T, A>, T, const T> {


		private:

			// -- private types ----------------------------------------------

			using self = ca::dynamic_array<T, A>;


			// -- private constants ------------------------------------------

			static constexpr ca::usz _initial_capacity = 4U;

			template <typename... Args>
			static constexpr bool _copies_before_realloc = ca::is_trivially_copyable<T>
				&& (sizeof...(Args) == 0U || (sizeof...(Args) == 1U && (... && ca::is_same<ca::remove_cvref<Args>, T>)));


		public:

			// -- public types -----------------------------------------------

			using value_type      = T;
			using allocator_type  = A;
			using size_type       = ca::usz;
			using reference       = T&;
			using const_reference = const T&;
			using pointer         = T*;
			using const_pointer   = const T*;
			using iterator        = T*;
			using const_iterator  = const T*;

			/* @DOC The same array type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::dynamic_array<T, B>;


			// -- public constants -------------------------------------------

			/* @DOC Three pointers and the allocator, no self-reference: containers move a
			   dynamic_array with memcpy when A allows it. */
			static constexpr bool TRIVIALLY_RELOCATABLE = ca::is_trivially_relocatable<A>;


		private:

			// -- private members --------------------------------------------

			T*                      _data;
			T*                      _end;
			T*                      _limit;
			[[no_unique_address]] A _allocator;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr dynamic_array(void) noexcept requires ca::is_stateless_allocator<A>
			: _data{nullptr}, _end{nullptr}, _limit{nullptr}, _allocator{} {
			}

			/* @DOC Empty, drawing memory from allocator. */
			constexpr explicit dynamic_array(const A& allocator) noexcept
			: _data{nullptr}, _end{nullptr}, _limit{nullptr}, _allocator{allocator} {
			}

			dynamic_array(const self&) = delete;

			constexpr dynamic_array(self&& other) noexcept
			: _data{ca::exchange(other._data, nullptr)}, _end{ca::exchange(other._end, nullptr)}, _limit{ca::exchange(other._limit, nullptr)},
			  _allocator{other._allocator} {
			}

			// Failsafe for a skipped cleanup(): destroys the elements and frees, silently.
			// ~dynamic_array(void) noexcept {
			// 	this->_release();
			// }


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Takes other's contents and allocator; the previous contents are released as by the
			   destructor. Traps when T is cleanable and elements remain: clear() or cleanup() first. */
			auto operator=(self&& other) noexcept -> self& {

				if (this == &other)
					return *this;
				if constexpr (ca::is_cleanable<T>) {
					if (_end != _data) [[unlikely]]
						ca::impl::assigned_over_uncleaned_elements();
				}
				this->_release();
				     _data = ca::exchange(other._data, nullptr);
				      _end = ca::exchange(other._end, nullptr);
				    _limit = ca::exchange(other._limit, nullptr);
				_allocator = other._allocator;
				return *this;
			}


			// -- iterators --------------------------------------------------

			[[nodiscard]] constexpr auto begin(void) noexcept -> T* {
				return _data;
			}

			[[nodiscard]] constexpr auto begin(void) const noexcept -> const T* {
				return _data;
			}

			[[nodiscard]] constexpr auto end(void) noexcept -> T* {
				return _end;
			}

			[[nodiscard]] constexpr auto end(void) const noexcept -> const T* {
				return _end;
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
				return static_cast<ca::usz>(_end - _data);
			}

			[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
				return static_cast<ca::usz>(_limit - _data);
			}

			[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
				return _end == _data;
			}

			[[nodiscard]] constexpr auto data(void) noexcept -> T* {
				return _data;
			}

			[[nodiscard]] constexpr auto data(void) const noexcept -> const T* {
				return _data;
			}

			[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
				return _allocator;
			}


			// -- element access ---------------------------------------------

			/* @DOC Element at index. Traps when index >= size(), in every build. */
			[[nodiscard]] constexpr auto operator[](const ca::usz index) noexcept -> T& {

				this->_expect_index(index);
				return _data[index];
			}

			[[nodiscard]] constexpr auto operator[](const ca::usz index) const noexcept -> const T& {

				this->_expect_index(index);
				return _data[index];
			}

			/* @DOC Element at index, or nothing when index >= size(). */
			[[nodiscard]] constexpr auto get(const ca::usz index) noexcept -> ca::optional<T&> {

				if (index >= this->size())
					return ca::nullopt;
				return ca::optional<T&>{_data[index]};
			}

			[[nodiscard]] constexpr auto get(const ca::usz index) const noexcept -> ca::optional<const T&> {

				if (index >= this->size())
					return ca::nullopt;
				return ca::optional<const T&>{_data[index]};
			}

			// -- modifiers --------------------------------------------------

			/* @DOC Makes room for count elements in total; never shrinks. On failure nothing changes. */
			[[nodiscard]] auto reserve(const ca::usz count) noexcept -> ca::result<void> requires ca::is_move_constructible<T> {

				if (count <= this->capacity())
					return ca::result<void>{};
				return this->_reallocate(count);
			}

			/* @DOC Brings capacity down to size(); frees the storage when empty. On failure nothing
			   changes. */
			[[nodiscard]] auto shrink(void) noexcept -> ca::result<void> requires ca::is_move_constructible<T> {

				if (_end == _limit)
					return ca::result<void>{};
				if (_end == _data) {
					this->_release();
					return ca::result<void>{};
				}
				return this->_reallocate(this->size());
			}

			/* @DOC Sets size() to count. Growing builds each new element from args, to capacity count
			   when full; on failure nothing changes. Shrinking cleans up and destroys the tail;
			   returns the first cleanup error, the tail is gone regardless. */
			template <typename... Args>
			requires ca::is_constructible<T, const Args&...> && ca::is_move_constructible<T>
			[[nodiscard]] auto resize(const ca::usz count, const Args&... args) noexcept -> ca::result<void> {

				const ca::usz size = this->size();
				if (count <= size)
					return this->_truncate(count);
				const ca::usz gap = count - size;
				if (count > this->capacity()) {
					if constexpr (sizeof...(Args) == 0U) {
						auto grown = this->_reallocate(count);
						if (!grown)
							return grown;
						T* const tail = _end;
						for (ca::usz i = 0U; i < gap; ++i)
							ca::construct_at(tail + i);
						_end = tail + gap;
						return ca::result<void>{};
					}
					else if constexpr (self::_copies_before_realloc<Args...>) {
						const T value(args...);
						auto grown = this->_reallocate(count);
						if (!grown)
							return grown;
						T* const tail = _end;
						for (ca::usz i = 0U; i < gap; ++i)
							ca::construct_at(tail + i, value);
						_end = tail + gap;
						return ca::result<void>{};
					}
					else
						return this->_regrow(count, size, gap, [&](T* const at) {
							for (ca::usz i = 0U; i < gap; ++i)
								ca::construct_at(at + i, args...);
						});
				}
				T* const tail = _end;
				for (ca::usz i = 0U; i < gap; ++i)
					ca::construct_at(tail + i, args...);
				_end = tail + gap;
				return ca::result<void>{};
			}

			/* @DOC Appends a T built from args, growing when full. On failure nothing changes. */
			template <typename... Args>
			requires ca::is_constructible<T, Args...> && ca::is_move_constructible<T>
			[[nodiscard]] auto emplace(Args&&... args) noexcept -> ca::result<void> {

				if (_end == _limit) [[unlikely]] {
					if constexpr (self::_copies_before_realloc<Args...>) {
						T value(ca::forward<Args>(args)...);
						auto grown = this->_reallocate(this->_next_capacity());
						if (!grown)
							return grown;
						ca::construct_at(_end, ca::move(value));
						++_end;
						return ca::result<void>{};
					}
					else {
						const ca::usz size = this->size();
						auto block = self::_appended_block(_allocator, _data, size, this->capacity(), this->_next_capacity(), ca::forward<Args>(args)...);
						if (!block)
							return block.propagate();
						this->_adopt(*block, size + 1U);
						return ca::result<void>{};
					}
				}
				ca::construct_at(_end, ca::forward<Args>(args)...);
				++_end;
				return ca::result<void>{};
			}

			/* @DOC emplace(value): list.append. */
			[[nodiscard]] auto append(const T& value) noexcept -> ca::result<void> requires ca::is_copy_constructible<T> && ca::is_move_constructible<T> {
				return this->emplace(value);
			}

			[[nodiscard]] auto append(T&& value) noexcept -> ca::result<void> requires ca::is_move_constructible<T> {
				return this->emplace(ca::move(value));
			}

			/* @DOC Copies the elements of view to the end, growing geometrically: list.extend. view
			   must not be into this array. On failure nothing changes. */
			[[nodiscard]] auto extend(const ca::borrowed_slice<const T> view) noexcept -> ca::result<void> requires ca::is_copy_constructible<T> && ca::is_move_constructible<T> {

				const ca::usz size = this->size();
				const ca::usz count = view.size();
				ca::usz total{0U};
				if (ca::add_overflow(total, size, count)) [[unlikely]]
					return ca::failure{ca::errc::out_of_memory};
				if (total > this->capacity()) {
					auto grown = this->_reallocate(ca::max(total, this->_next_capacity()));
					if (!grown)
						return grown;
				}
				ca::copy_construct_n(view.data(), count, _end);
				_end += count;
				return ca::result<void>{};
			}

			/* @DOC Builds a T from args at index, shifting the elements after it, growing when full.
			   Traps when index > size(), in every build. On failure nothing changes. */
			template <typename... Args>
			requires ca::is_constructible<T, Args...> && ca::is_move_constructible<T>
			[[nodiscard]] auto insert(const ca::usz index, Args&&... args) noexcept -> ca::result<void> {

				const ca::usz size = this->size();
				if (index > size) [[unlikely]]
					ca::impl::index_out_of_bounds();
				if (index == size)
					return this->emplace(ca::forward<Args>(args)...);
				if (_end == _limit) [[unlikely]]
					return this->_regrow(this->_next_capacity(), index, 1U, [&](T* const at) {
						ca::construct_at(at, ca::forward<Args>(args)...);
					});
				T value(ca::forward<Args>(args)...);
				ca::relocate_overlapping_n(_data + index, size - index, _data + index + 1U);
				ca::construct_at(_data + index, ca::move(value));
				++_end;
				return ca::result<void>{};
			}

			/* @DOC Copies the elements of view in at index, shifting the elements after it, growing
			   geometrically. view must not be into this array. Traps when index > size(), in every
			   build. On failure nothing changes. */
			[[nodiscard]] auto insert(const ca::usz index, const ca::borrowed_slice<const T> view) noexcept -> ca::result<void>
			requires ca::is_copy_constructible<T> && ca::is_move_constructible<T> {

				const ca::usz size = this->size();
				if (index > size) [[unlikely]]
					ca::impl::index_out_of_bounds();
				const ca::usz count = view.size();
				ca::usz total{0U};
				if (ca::add_overflow(total, size, count)) [[unlikely]]
					return ca::failure{ca::errc::out_of_memory};
				if (total > this->capacity())
					return this->_regrow(ca::max(total, this->_next_capacity()), index, count, [&](T* const at) {
						ca::copy_construct_n(view.data(), count, at);
					});
				ca::relocate_overlapping_n(_data + index, size - index, _data + index + count);
				ca::copy_construct_n(view.data(), count, _data + index);
				_end += count;
				return ca::result<void>{};
			}

			/* @DOC Removes and returns the last element, or nothing when empty. */
			[[nodiscard]] auto pop(void) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {

				if (_end == _data)
					return ca::nullopt;
				--_end;
				ca::optional<T> last{ca::in_place, ca::move(*_end)};
				ca::destroy_at(_end);
				return last;
			}

			/* @DOC Removes and returns the element at index, shifting the elements after it, or
			   nothing when index >= size(): list.pop(i). */
			[[nodiscard]] auto pop(const ca::usz index) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {

				if (index >= this->size())
					return ca::nullopt;
				ca::optional<T> removed{ca::in_place, ca::move(_data[index])};
				ca::destroy_at(_data + index);
				ca::relocate_overlapping_n(_data + index + 1U, this->size() - index - 1U, _data + index);
				--_end;
				return removed;
			}

			/* @DOC Removes and returns the first element equal to value, shifting the elements after
			   it, or nothing when none is: list.remove. */
			[[nodiscard]] auto remove(const T& value) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {

				const ca::optional<ca::usz> found = this->find(value);
				if (!found)
					return ca::nullopt;
				return this->pop(*found);
			}

			/* @DOC Removes and returns the element at index in O(1), the last element taking its
			   place, or nothing when index >= size(). Does not keep order. */
			[[nodiscard]] auto swap_pop(const ca::usz index) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {

				if (index >= this->size())
					return ca::nullopt;
				ca::optional<T> removed{ca::in_place, ca::move(_data[index])};
				ca::destroy_at(_data + index);
				--_end;
				if (_data + index != _end)
					ca::relocate_n(_end, 1U, _data + index);
				return removed;
			}

			/* @DOC Cleans up and destroys every element; keeps the capacity. Returns the first cleanup
			   error; every element is gone regardless. */
			[[nodiscard]] auto clear(void) noexcept -> ca::result<void> {
				return this->_truncate(0U);
			}

			/* @DOC Cleans up every element (cleanup() when T has one), then frees the storage: empty,
			   no capacity. Returns the first cleanup error; everything is released regardless. */
			[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

				auto cleaned = this->_truncate(0U);
				this->_release();
				return cleaned;
			}


			// -- methods ----------------------------------------------------

			/* @DOC The elements, as a view: what the shared methods work on. */
			[[nodiscard]] constexpr auto view(void) noexcept -> ca::borrowed_slice<T> {
				return ca::borrowed_slice<T>{_data, this->size()};
			}

			[[nodiscard]] constexpr auto view(void) const noexcept -> ca::borrowed_slice<const T> {
				return ca::borrowed_slice<const T>{_data, this->size()};
			}

			/* @DOC The elements as a fixed-length slice, block and allocator handed over as they are:
			   no copy, no allocation, the spare capacity kept. The array is left empty. */
			[[nodiscard]] auto into_slice(void) && noexcept -> ca::slice<T, A> {

				ca::slice<T, A> fixed{_allocator};
				 fixed._data = ca::exchange(_data, nullptr);
				  fixed._end = ca::exchange(_end, nullptr);
				fixed._limit = ca::exchange(_limit, nullptr);
				return fixed;
			}

			/* @DOC A copy in memory from the same allocator, with capacity size() or what the
			   allocator rounds it to. Elements are copied with clone() when T has one, else with the
			   copy constructor. On a failed element clone, the partial copy is destroyed and that
			   error returned. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self> requires ca::is_clonable<T> || ca::is_copy_constructible<T> {
				return this->clone_in(_allocator);
			}

			/* @DOC clone, in memory from allocator. */
			[[nodiscard]] auto clone_in(const A& allocator) const noexcept -> ca::result<self> requires ca::is_clonable<T> || ca::is_copy_constructible<T> {

				self copy{allocator};
				const ca::usz size = this->size();
				if (size == 0U)
					return copy;
				auto block = ca::allocate_n<T>(allocator, size);
				if (!block)
					return block.propagate();
				copy._adopt(*block, 0U);
				if constexpr (ca::is_clonable<T>) {
					for (const T& element : *this) {
						auto cloned = element.clone();
						if (!cloned) {
							static_cast<void>(copy.cleanup());
							return cloned.propagate();
						}
						ca::construct_at(copy._end, ca::move(*cloned));
						++copy._end;
					}
				}
				else {
					ca::copy_construct_n(_data, size, copy._data);
					copy._end = copy._data + size;
				}
				return copy;
			}


		private:

			// -- private methods --------------------------------------------

			constexpr auto _expect_index(const ca::usz index) const noexcept -> void {
				if (index >= this->size()) [[unlikely]]
					ca::impl::index_out_of_bounds();
			}

			auto _next_capacity(void) const noexcept -> ca::usz {

				ca::usz doubled{0U};
				if (ca::mul_overflow(doubled, this->capacity(), 2U))
					doubled = ca::limits<ca::usz>::max();
				return ca::max(doubled, self::_initial_capacity);
			}

			[[nodiscard]] static auto _block(T* const data, const ca::usz capacity) noexcept -> ca::allocation {
				return ca::allocation{data, capacity * sizeof(T)};
			}

			[[nodiscard, gnu::noinline]] static auto _new_block(const A allocator, const ca::usz count) noexcept -> ca::result<ca::allocation> {
				return ca::allocate_n<T>(allocator, count);
			}

			[[gnu::always_inline]] static auto _free_block(const A allocator, T* const data, const ca::usz capacity) noexcept -> void {
				ca::deallocate_n<T>(allocator, self::_block(data, capacity));
			}

			[[nodiscard, gnu::noinline]] static auto _moved_block(const A allocator, T* const data, const ca::usz size, const ca::usz capacity, const ca::usz count) noexcept
			-> ca::result<ca::allocation> {
				if constexpr (ca::is_trivially_relocatable<T>)
					return ca::reallocate_n<T>(allocator, self::_block(data, capacity), count);
				else {
					auto expanded = ca::expand_n<T>(allocator, self::_block(data, capacity), count);
					if (expanded)
						return *expanded;
					auto moved = ca::allocate_n<T>(allocator, count);
					if (!moved)
						return moved.propagate();
					ca::relocate_n(data, size, static_cast<T*>(moved->data()));
					ca::deallocate_n<T>(allocator, self::_block(data, capacity));
					return moved;
				}
			}

			[[gnu::noinline]] static auto _relocate_around(const A allocator, T* const data, const ca::usz size, const ca::usz capacity, const ca::usz index, const ca::usz gap, T* const into) noexcept
			-> void {

				ca::relocate_n(data, index, into);
				ca::relocate_n(data + index, size - index, into + index + gap);
				ca::deallocate_n<T>(allocator, self::_block(data, capacity));
			}

			template <typename... Args>
			[[nodiscard, gnu::noinline]] static auto _appended_block(const A allocator, T* const data, const ca::usz size, const ca::usz capacity, const ca::usz count, Args&&... args) noexcept
			-> ca::result<ca::allocation> {

				auto appended = ca::allocate_n<T>(allocator, count);
				if (!appended)
					return appended.propagate();
				T* const into = static_cast<T*>(appended->data());
				ca::construct_at(into + size, ca::forward<Args>(args)...);
				if (data != nullptr)
					self::_relocate_around(allocator, data, size, capacity, size, 1U, into);
				return appended;
			}

			auto _adopt(const ca::allocation block, const ca::usz size) noexcept -> void {

				T* const data = static_cast<T*>(block.data());
				 _data = data;
				  _end = data + size;
				_limit = data + block.size() / sizeof(T);
			}

			[[nodiscard, gnu::always_inline]] auto _reallocate(const ca::usz count) noexcept -> ca::result<void> {

				const ca::usz size = this->size();
				auto block = _data == nullptr ? self::_new_block(_allocator, count) : self::_moved_block(_allocator, _data, size, this->capacity(), count);
				if (!block)
					return block.propagate();
				this->_adopt(*block, size);
				return ca::result<void>{};
			}

			template <typename Build>
			[[nodiscard, gnu::always_inline]] auto _regrow(const ca::usz count, const ca::usz index, const ca::usz gap, Build&& build) noexcept -> ca::result<void> {

				const ca::usz size = this->size();
				auto block = self::_new_block(_allocator, count);
				if (!block)
					return block.propagate();
				T* const into = static_cast<T*>(block->data());
				build(into + index);
				if (_data != nullptr)
					self::_relocate_around(_allocator, _data, size, this->capacity(), index, gap, into);
				this->_adopt(*block, size + gap);
				return ca::result<void>{};
			}

			[[nodiscard]] auto _truncate(const ca::usz count) noexcept -> ca::result<void> {

				T* const tail = _data + count;
				if constexpr (ca::is_cleanable<T>) {
					ca::error first{};
					for (T* element = tail; element != _end; ++element) {
						element->cleanup().match([] {
						}, [&first](const ca::error& error) {
							if (first.ok())
								first = error;
						});
						ca::destroy_at(element);
					}
					_end = tail;
					if (!first.ok())
						return ca::failure{first};
				}
				else {
					ca::destroy_n(tail, static_cast<ca::usz>(_end - tail));
					_end = tail;
				}
				return ca::result<void>{};
			}

			auto _release(void) noexcept -> void {

				if (_data == nullptr)
					return;
				ca::destroy_n(_data, this->size());
				self::_free_block(_allocator, _data, this->capacity());
				 _data = nullptr;
				  _end = nullptr;
				_limit = nullptr;
			}


			// -- assertions -------------------------------------------------

			static_assert(ca::is_object<T> && !ca::is_const<T> && !ca::is_array<T>,
				"ca::dynamic_array<T>: T must be a non-const, non-array object type");

	}; // class dynamic_array

} // namespace ca

#endif // ca_collections_array_dynamic_array_hpp
