/* @DOC Performance of ca::small_array<T, N, A>.

   Layout: three pointers (data, end, limit), the allocator (nothing for a stateless one), then
   room for N elements: small_array<int, 4> is 40 bytes on 64-bit. While size() <= N the
   elements live in that room and data points into the object; past N they move to a block from
   A, as in dynamic_array, and the room goes unused until shrink() brings them back.

   | Operation | Time | Notes |
   |---|---|---|
   | operator[], get, first, last | O(1) | one compare; data is a pointer either way, no inline/heap branch |
   | size, capacity, empty, is_inline, begin, end, data | O(1) | |
   | append, emplace | amortized O(1) | the first N cost no allocation; leaving the inline room is one allocation and one relocation |
   | extend(view) | amortized O(n) | one memcpy for trivially copyable T |
   | insert(i), pop(i) | O(n - i) | one memmove for trivially relocatable T |
   | swap_pop(i), pop() | O(1) | |
   | reserve | O(n) | on the heap, trivially relocatable T: ca::reallocate |
   | shrink | O(n) | back into the inline room when size() <= N |
   | move construction, move assignment | O(1) on the heap, O(size) inline | inline elements are relocated: one memcpy for trivially relocatable T |
   | clear, cleanup, clone | O(n) | |
   | find, count, contains, compare, == | O(n) | memchr/memcmp for code units, element loops otherwise |

   Space: N * sizeof(T) more than dynamic_array, always; growth past N doubles from N, the
   allocator's rounding included. Pick N as the size most instances never pass: then most
   instances never allocate.

   Locality: inline elements sit next to the header, in the enclosing object or on the stack; a
   small_array member of a node puts the node's short list in the node's own cache lines.

   Relocation: not TRIVIALLY_RELOCATABLE, since data points into the object while inline:
   containers move it with its move constructor, which re-points data. Elements move as in
   dynamic_array.

   Copy and move: the copy constructor is deleted; clone() is the explicit, fallible copy, inline
   when it fits. Moving leaves the source empty and inline.

   Inlining: the fast paths (append, emplace, operator[], pop) are those of dynamic_array; growth
   runs in out-of-line static helpers that receive pointers, sizes and the allocator by value.

   Against std::vector: no allocation at all for the first N elements; past N the same costs.

   Baseline std::vector<int> over glibc malloc; ca is small_array<int, 8> over ca::heap:
   --BENCH bench/small_arrays.cpp small_array
   bench/small_arrays.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | 1e6 times build 4 elements (inline) and drop | 0.092x | 0.064x | |
   | 1e6 times build 32 elements (leaves the inline room) and drop | 0.43x | 0.40x | |
   | churn, 1e7 appends, popping all at 8 | 0.82x | 1.57x | gcc: vectorized pops stall on store forwarding |
   BENCH--
   An always-inline growth step keeps `this` from escaping; without it clang was 1.03x on the
   churn. */

#ifndef ca_collections_array_small_array_hpp
#define ca_collections_array_small_array_hpp

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
#include "1-primitives/traits/is_trivially_destructible.hpp"
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


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SMALL ARRAY --------------------------------------------------------

	/* @DOC Growable array of T keeping its first N elements in the object itself, then moving to
	   memory from A: LLVM's SmallVector. Short instances never allocate; long ones behave as a
	   dynamic_array. reserve, append, extend, emplace, insert, resize and clone acquire memory
	   once past N and can fail, changing nothing then. Has the sequence methods (cut, find,
	   compare, fill, reverse) of ca::impl::slice_methods. Clean up with cleanup(); the destructor
	   is a failsafe that frees silently. operator[] traps out of range; get allows the doubt. A
	   stateful A has no default construction and moves with the elements. */
	template <typename T, ca::usz N, ca::is_allocator A = ca::heap>
	class small_array final : public ca::impl::slice_methods<ca::small_array<T, N, A>, T, const T> {


		private:

			// -- private types ----------------------------------------------

			using self = ca::small_array<T, N, A>;


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
			using rebind = ca::small_array<T, N, B>;


			// -- public constants -------------------------------------------

			/* @DOC Elements held without allocating. */
			static constexpr ca::usz INLINE_CAPACITY = N;


		private:

			// -- private members --------------------------------------------

			T*                      _data;
			T*                      _end;
			T*                      _limit;
			[[no_unique_address]] A _allocator;
			union {
					T _slots[N];
			};


		public:

			// -- public lifecycle -------------------------------------------

			small_array(void) noexcept requires ca::is_stateless_allocator<A>
			: _data{_slots}, _end{_slots}, _limit{_slots + N}, _allocator{} {
			}

			/* @DOC Empty and inline; memory past N comes from allocator. */
			explicit small_array(const A& allocator) noexcept
			: _data{_slots}, _end{_slots}, _limit{_slots + N}, _allocator{allocator} {
			}

			small_array(const self&) = delete;

			/* @DOC Takes other's block, or relocates its inline elements; other is left empty and
			   inline. */
			small_array(self&& other) noexcept requires ca::is_move_constructible<T>
			: _data{_slots}, _end{_slots}, _limit{_slots + N}, _allocator{other._allocator} {
				this->_take(other);
			}

			~small_array(void) noexcept requires ca::is_trivially_destructible<T> = default;

			~small_array(void) noexcept {
			}

			// Failsafe for a skipped cleanup(): destroys the elements and frees, silently.
			// ~small_array(void) noexcept {
			// 	this->_release();
			// }


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Takes other's contents and allocator; the previous contents are released as by the
			   destructor. Traps when T is cleanable and elements remain: clear() or cleanup() first. */
			auto operator=(self&& other) noexcept -> self& requires ca::is_move_constructible<T> {

				if (this == &other)
					return *this;
				if constexpr (ca::is_cleanable<T>) {
					if (_end != _data) [[unlikely]]
						ca::impl::assigned_over_uncleaned_elements();
				}
				this->_release();
				_allocator = other._allocator;
				this->_take(other);
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

			/* @DOC N while inline, the block's room once on the heap. */
			[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
				return static_cast<ca::usz>(_limit - _data);
			}

			[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
				return _end == _data;
			}

			/* @DOC The elements are in the object itself; nothing is allocated. */
			[[nodiscard]] constexpr auto is_inline(void) const noexcept -> bool {
				return _data == _slots;
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

			/* @DOC Brings the elements back inline when size() <= N, freeing the block; else brings
			   capacity down to size(). On failure nothing changes. */
			[[nodiscard]] auto shrink(void) noexcept -> ca::result<void> requires ca::is_move_constructible<T> {

				if (this->is_inline() || _end == _limit)
					return ca::result<void>{};
				const ca::usz size = this->size();
				if (size <= N) {
					T* const data = _data;
					const ca::usz capacity = this->capacity();
					ca::relocate_n(data, size, _slots);
					self::_free_block(_allocator, data, capacity);
					 _data = _slots;
					  _end = _slots + size;
					_limit = _slots + N;
					return ca::result<void>{};
				}
				return this->_reallocate(size);
			}

			/* @DOC Sets size() to count. Growing builds each new element from args (value-initialized
			   without), to capacity count when full; on failure nothing changes. Shrinking cleans up
			   and destroys the tail; returns the first cleanup error, the tail is gone regardless. */
			template <typename... Args>
			requires ca::is_constructible<T, const Args&...> && ca::is_move_constructible<T>
			[[nodiscard]] auto resize(const ca::usz count, const Args&... args) noexcept -> ca::result<void> {

				const ca::usz size = this->size();
				if (count <= size)
					return this->_truncate(count);
				if (count > this->capacity()) {
					if constexpr (sizeof...(Args) == 0U) {
						auto grown = this->_reallocate(count);
						if (!grown)
							return grown;
					}
					else {
						const T value(args...);
						auto grown = this->_reallocate(count);
						if (!grown)
							return grown;
						T* const tail = _end;
						for (ca::usz i = 0U; i < count - size; ++i)
							ca::construct_at(tail + i, value);
						_end = tail + (count - size);
						return ca::result<void>{};
					}
				}
				T* const tail = _end;
				for (ca::usz i = 0U; i < count - size; ++i)
					ca::construct_at(tail + i, args...);
				_end = tail + (count - size);
				return ca::result<void>{};
			}

			/* @DOC Appends a T built from args, leaving the inline room or growing when full. args
			   may refer to an element. On failure nothing changes. */
			template <typename... Args>
			requires ca::is_constructible<T, Args...> && ca::is_move_constructible<T>
			[[nodiscard]] auto emplace(Args&&... args) noexcept -> ca::result<void> {

				if (_end == _limit) [[unlikely]] {
					T value(ca::forward<Args>(args)...);
					auto grown = this->_reallocate(this->_next_capacity());
					if (!grown)
						return grown;
					ca::construct_at(_end, ca::move(value));
					++_end;
					return ca::result<void>{};
				}
				ca::construct_at(_end, ca::forward<Args>(args)...);
				++_end;
				return ca::result<void>{};
			}

			/* @DOC emplace(value). */
			[[nodiscard]] auto append(const T& value) noexcept -> ca::result<void> requires ca::is_copy_constructible<T> && ca::is_move_constructible<T> {
				return this->emplace(value);
			}

			[[nodiscard]] auto append(T&& value) noexcept -> ca::result<void> requires ca::is_move_constructible<T> {
				return this->emplace(ca::move(value));
			}

			/* @DOC Copies the elements of view to the end, growing geometrically. view must not be
			   into this array. On failure nothing changes. */
			[[nodiscard]] auto extend(const ca::borrowed_slice<const T> view) noexcept -> ca::result<void> requires ca::is_copy_constructible<T> && ca::is_move_constructible<T> {

				const ca::usz count = view.size();
				ca::usz total{0U};
				if (ca::add_overflow(total, this->size(), count)) [[unlikely]]
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
			   args may refer to an element. Traps when index > size(), in every build. On failure
			   nothing changes. */
			template <typename... Args>
			requires ca::is_constructible<T, Args...> && ca::is_move_constructible<T>
			[[nodiscard]] auto insert(const ca::usz index, Args&&... args) noexcept -> ca::result<void> {

				const ca::usz size = this->size();
				if (index > size) [[unlikely]]
					ca::impl::index_out_of_bounds();
				T value(ca::forward<Args>(args)...);
				if (_end == _limit) [[unlikely]] {
					auto grown = this->_reallocate(this->_next_capacity());
					if (!grown)
						return grown;
				}
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
				if (total > this->capacity()) {
					auto grown = this->_reallocate(ca::max(total, this->_next_capacity()));
					if (!grown)
						return grown;
				}
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
			   nothing when index >= size(). */
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

			/* @DOC Cleans up every element (cleanup() when T has one), then frees the block: empty and
			   inline. Returns the first cleanup error; everything is released regardless. */
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

			/* @DOC A copy with memory from the same allocator, inline when size() <= N. Elements are
			   copied with clone() when T has one, else with the copy constructor. On a failed
			   element clone, the partial copy is destroyed and that error returned. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self>
			requires (ca::is_clonable<T> || ca::is_copy_constructible<T>) && ca::is_move_constructible<T> {
				return this->clone_in(_allocator);
			}

			/* @DOC clone, with memory from allocator. */
			[[nodiscard]] auto clone_in(const A& allocator) const noexcept -> ca::result<self>
			requires (ca::is_clonable<T> || ca::is_copy_constructible<T>) && ca::is_move_constructible<T> {

				self copy{allocator};
				auto reserved = copy.reserve(this->size());
				if (!reserved)
					return reserved.propagate();
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
					ca::copy_construct_n(_data, this->size(), copy._data);
					copy._end = copy._data + this->size();
				}
				return copy;
			}


		private:

			// -- private methods --------------------------------------------

			constexpr auto _expect_index(const ca::usz index) const noexcept -> void {
				if (index >= this->size()) [[unlikely]]
					ca::impl::index_out_of_bounds();
			}

			[[nodiscard]] auto _next_capacity(void) const noexcept -> ca::usz {

				ca::usz doubled{0U};
				if (ca::mul_overflow(doubled, this->capacity(), 2U))
					doubled = ca::limits<ca::usz>::max();
				return doubled;
			}

			[[nodiscard]] static auto _block(T* const data, const ca::usz capacity) noexcept -> ca::allocation {
				return ca::allocation{data, capacity * sizeof(T)};
			}

			[[gnu::always_inline]] static auto _free_block(const A allocator, T* const data, const ca::usz capacity) noexcept -> void {
				ca::deallocate_n<T>(allocator, self::_block(data, capacity));
			}

			[[nodiscard, gnu::noinline]] static auto _out_of_line(const A allocator, T* const data, const ca::usz size, const ca::usz count) noexcept -> ca::result<ca::allocation> {

				auto moved = ca::allocate_n<T>(allocator, count);
				if (!moved)
					return moved.propagate();
				ca::relocate_n(data, size, static_cast<T*>(moved->data()));
				return moved;
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

			[[nodiscard, gnu::always_inline]] auto _reallocate(const ca::usz count) noexcept -> ca::result<void> {

				const ca::usz size = this->size();
				auto block = this->is_inline() ? self::_out_of_line(_allocator, _data, size, count) : self::_moved_block(_allocator, _data, size, this->capacity(), count);
				if (!block)
					return block.propagate();
				T* const data = static_cast<T*>(block->data());
				 _data = data;
				  _end = data + size;
				_limit = data + block->size() / sizeof(T);
				return ca::result<void>{};
			}

			auto _take(self& other) noexcept -> void {

				if (other.is_inline()) {
					const ca::usz size = other.size();
					ca::relocate_n(other._data, size, _slots);
					 _data = _slots;
					  _end = _slots + size;
					_limit = _slots + N;
				}
				else {
					 _data = other._data;
					  _end = other._end;
					_limit = other._limit;
				}
				 other._data = other._slots;
				  other._end = other._slots;
				other._limit = other._slots + N;
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

				ca::destroy_n(_data, this->size());
				if (!this->is_inline())
					self::_free_block(_allocator, _data, this->capacity());
				 _data = _slots;
				  _end = _slots;
				_limit = _slots + N;
			}


			// -- assertions -------------------------------------------------

			static_assert(ca::is_object<T> && !ca::is_const<T> && !ca::is_array<T>,
				"ca::small_array<T, N>: T must be a non-const, non-array object type");

			static_assert(N != 0U, "ca::small_array<T, N>: N must not be 0; use dynamic_array");

	}; // class small_array

} // namespace ca

#endif // ca_collections_array_small_array_hpp
