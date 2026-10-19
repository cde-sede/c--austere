/* @DOC Performance of ca::deque<T, A>.

   Layout: a pointer to a ring of capacity() slots and three integers (head slot, size,
   capacity), 32 bytes on 64-bit, plus the allocator: nothing for a stateless one (ca::heap,
   the default). Capacity is a power of two, so element i lives at (head + i) & (capacity - 1):
   one add and one and, no division, no branch. An empty deque owns no memory.

   | Operation | Time | Notes |
   |---|---|---|
   | operator[], get, first, last | O(1) | one compare, one add, one and |
   | size, capacity, empty, begin, end | O(1) | |
   | append, emplace, prepend, emplace_front | amortized O(1) | O(n) on the growth step |
   | pop, pop_front | O(1) | one emptiness compare |
   | extend(view) | amortized O(n) | at most two memcpy for trivially copyable T |
   | reserve | O(n) | trivially relocatable T: ca::reallocate, then the shorter run (at most half) moves once |
   | shrink, clone | O(n) | elements laid out from slot 0 |
   | clear, cleanup | O(n) | O(1) for trivially destructible T without cleanup() |
   | as_slices | O(1) | the two contiguous runs, front first |
   | find, count, contains | O(n) | over the two runs |

   Space: capacity doubles, the first allocation holds 4 elements; capacity is the power of two
   asked for, so up to 2x the live elements after growth (plus the allocator's rounding, unused).
   shrink() brings it to the power of two at or above size().

   Locality and alignment: elements are contiguous in at most two runs (as_slices); a
   queue that never holds more than capacity() elements cycles through one block forever, with
   no allocation and no shifting. Iteration masks each index; loops over as_slices() vectorize.

   Relocation: growth by ca::reallocate (trivially relocatable T) or expand leaves the block's
   contents in place and relocates only the shorter run: the wrapped one past the old capacity,
   or the front one to the end of the new block. One memcpy of at most half the elements. A fresh
   block takes both runs at those same offsets. Other types are move-constructed and destroyed
   one by one.

   Aliasing: unlike dynamic_array, which stores pointers, the head and size are integers; a store
   of usz-sized integer elements may make the compiler reload them. Growth builds the new element
   before the block moves, so `d.append(d[0])` stays valid; a failed growth changes nothing.

   Copy and move: the copy constructor is deleted; clone() is the explicit, fallible copy. Move
   construction and assignment take four words and the allocator, noexcept, no allocation.
   TRIVIALLY_RELOCATABLE when A is.

   Inlining: append, prepend, pop, pop_front and operator[] are a handful of instructions; growth
   runs in out-of-line static helpers that receive pointers, sizes and the allocator by value.

   Against std::deque (a map of fixed 512-byte blocks): one block instead of one per 512 bytes and
   a map, element access without the two-level lookup, contiguous runs for bulk work. std::deque
   never moves elements on growth, which keeps references stable; ca::deque does not promise that.

   Baseline std::deque; ca over ca::heap:
   --BENCH bench/deque.cpp
   bench/deque.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | append 1e7 ints, growing | 0.34x | 0.24x | |
   | prepend 1e7 ints, growing | 0.56x | 0.83x | |
   | FIFO 1e3 deep, 1e7 append + pop_front | 0.50x | 0.28x | |
   | both ends, 1e7 u64 | 0.74x | 0.56x | |
   | operator[] sum, 1e6 ints x 10 | 0.30x | 0.37x | one mask per index; std: a block lookup |
   | range-for sum, 1e6 ints x 10 | 0.67x | 0.71x | |
   BENCH--
   Growth must stay a static out-of-line helper fed by value: a member call passing `this`
   kept every field in memory (1.3x slower than std on clang), and inlining the whole growth
   path made clang stop inlining emplace (3x slower). */

#ifndef ca_collections_array_deque_hpp
#define ca_collections_array_deque_hpp

#include "1-primitives/bit/bit_ceil.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/limits.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/copy_construct_n.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/memory/relocate_n.hpp"
#include "1-primitives/numeric/add_overflow.hpp"
#include "1-primitives/numeric/max.hpp"
#include "1-primitives/numeric/min.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_move_constructible.hpp"
#include "1-primitives/traits/is_object.hpp"
#include "1-primitives/traits/remove_cv.hpp"
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
#include "5-collections/array/array.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DEQUE --------------------------------------------------------------

	namespace impl {

		/* @DOC Walks a deque in order: a logical index masked into the ring on access. */
		template <typename T>
		class deque_iterator final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::deque_iterator<T>;


			public:

				// -- public types -------------------------------------------

				using value_type = ca::remove_cv<T>;
				using reference  = T&;
				using pointer    = T*;


			private:

				// -- private members ----------------------------------------

				T*      _data;
				ca::usz _mask;
				ca::usz _index;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr deque_iterator(T* const data, const ca::usz mask, const ca::usz index) noexcept
				: _data{data}, _mask{mask}, _index{index} {
				}


				// -- element access -----------------------------------------

				[[nodiscard]] constexpr auto operator*(void) const noexcept -> T& {
					return _data[_index & _mask];
				}

				[[nodiscard]] constexpr auto operator->(void) const noexcept -> T* {
					return _data + (_index & _mask);
				}


				// -- methods ------------------------------------------------

				constexpr auto operator++(void) noexcept -> self& {

					++_index;
					return *this;
				}

				constexpr auto operator--(void) noexcept -> self& {

					--_index;
					return *this;
				}

				[[nodiscard]] constexpr auto operator==(const self& other) const noexcept -> bool {
					return _index == other._index;
				}

		}; // class deque_iterator

	} // namespace impl


	/* @DOC Growable double-ended queue of T in one ring of memory from A: Python's
	   collections.deque, Rust's VecDeque. append/prepend and pop/pop_front in O(1) at both ends,
	   operator[] in O(1). Construction allocates nothing; reserve, append, prepend, extend,
	   shrink and clone acquire memory and can fail, changing nothing then. Elements live in at
	   most two contiguous runs (as_slices). Clean up with cleanup(); the destructor is a failsafe
	   that frees silently. operator[] traps out of range; get allows the doubt. A stateful A has
	   no default construction and moves with the elements. */
	template <typename T, ca::is_allocator A = ca::heap>
	class deque final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::deque<T, A>;


			// -- private constants ------------------------------------------

			static constexpr ca::usz _initial_capacity = 4U;


		public:

			// -- public types -----------------------------------------------

			using value_type      = T;
			using allocator_type  = A;
			using size_type       = ca::usz;
			using reference       = T&;
			using const_reference = const T&;
			using pointer         = T*;
			using const_pointer   = const T*;
			using iterator        = ca::impl::deque_iterator<T>;
			using const_iterator  = ca::impl::deque_iterator<const T>;

			/* @DOC The same deque type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::deque<T, B>;


			// -- public constants -------------------------------------------

			/* @DOC A pointer, three integers and the allocator, no self-reference: containers move a
			   deque with memcpy when A allows it. */
			static constexpr bool TRIVIALLY_RELOCATABLE = ca::is_trivially_relocatable<A>;


		private:

			// -- private members --------------------------------------------

			T*                      _data;
			ca::usz                 _head;
			ca::usz                 _size;
			ca::usz                 _capacity;
			[[no_unique_address]] A _allocator;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr deque(void) noexcept requires ca::is_stateless_allocator<A>
			: _data{nullptr}, _head{0U}, _size{0U}, _capacity{0U}, _allocator{} {
			}

			/* @DOC Empty, drawing memory from allocator. */
			constexpr explicit deque(const A& allocator) noexcept
			: _data{nullptr}, _head{0U}, _size{0U}, _capacity{0U}, _allocator{allocator} {
			}

			deque(const self&) = delete;

			constexpr deque(self&& other) noexcept
			: _data{ca::exchange(other._data, nullptr)}, _head{ca::exchange(other._head, 0U)}, _size{ca::exchange(other._size, 0U)},
			  _capacity{ca::exchange(other._capacity, 0U)}, _allocator{other._allocator} {
			}

			// Failsafe for a skipped cleanup(): destroys the elements and frees, silently.
			// ~deque(void) noexcept {
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
					if (_size != 0U) [[unlikely]]
						ca::impl::assigned_over_uncleaned_elements();
				}
				this->_release();
				     _data = ca::exchange(other._data, nullptr);
				     _head = ca::exchange(other._head, 0U);
				     _size = ca::exchange(other._size, 0U);
				 _capacity = ca::exchange(other._capacity, 0U);
				_allocator = other._allocator;
				return *this;
			}


			// -- iterators --------------------------------------------------

			[[nodiscard]] constexpr auto begin(void) noexcept -> iterator {
				return iterator{_data, _capacity - 1U, _head};
			}

			[[nodiscard]] constexpr auto begin(void) const noexcept -> const_iterator {
				return const_iterator{_data, _capacity - 1U, _head};
			}

			[[nodiscard]] constexpr auto end(void) noexcept -> iterator {
				return iterator{_data, _capacity - 1U, _head + _size};
			}

			[[nodiscard]] constexpr auto end(void) const noexcept -> const_iterator {
				return const_iterator{_data, _capacity - 1U, _head + _size};
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
				return _size;
			}

			/* @DOC Slots in the ring: 0 or a power of two. */
			[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
				return _capacity;
			}

			[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
				return _size == 0U;
			}

			[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
				return _allocator;
			}


			// -- element access ---------------------------------------------

			/* @DOC Element at index from the front. Traps when index >= size(), in every build. */
			[[nodiscard]] constexpr auto operator[](const ca::usz index) noexcept -> T& {

				this->_expect_index(index);
				return _data[this->_slot(index)];
			}

			[[nodiscard]] constexpr auto operator[](const ca::usz index) const noexcept -> const T& {

				this->_expect_index(index);
				return _data[this->_slot(index)];
			}

			/* @DOC Element at index from the front, or nothing when index >= size(). */
			[[nodiscard]] constexpr auto get(const ca::usz index) noexcept -> ca::optional<T&> {

				if (index >= _size)
					return ca::nullopt;
				return ca::optional<T&>{_data[this->_slot(index)]};
			}

			[[nodiscard]] constexpr auto get(const ca::usz index) const noexcept -> ca::optional<const T&> {

				if (index >= _size)
					return ca::nullopt;
				return ca::optional<const T&>{_data[this->_slot(index)]};
			}

			/* @DOC Front element, or nothing when empty. */
			[[nodiscard]] constexpr auto first(void) noexcept -> ca::optional<T&> {
				return this->get(0U);
			}

			[[nodiscard]] constexpr auto first(void) const noexcept -> ca::optional<const T&> {
				return this->get(0U);
			}

			/* @DOC Back element, or nothing when empty. */
			[[nodiscard]] constexpr auto last(void) noexcept -> ca::optional<T&> {
				return this->get(_size - 1U);
			}

			[[nodiscard]] constexpr auto last(void) const noexcept -> ca::optional<const T&> {
				return this->get(_size - 1U);
			}


			// -- modifiers --------------------------------------------------

			/* @DOC Makes room for count elements in total, rounded up to a power of two; never
			   shrinks. On failure nothing changes. */
			[[nodiscard]] auto reserve(const ca::usz count) noexcept -> ca::result<void> requires ca::is_move_constructible<T> {

				if (count <= _capacity)
					return ca::result<void>{};
				if (count > ca::limits<ca::usz>::max() / 2U + 1U) [[unlikely]]
					return ca::failure{ca::errc::out_of_memory};
				return this->_grow(ca::bit_ceil(count));
			}

			/* @DOC Brings capacity down to the power of two at or above size(), into a new block laid
			   out from slot 0; frees the storage when empty, does nothing when capacity is already
			   that. On failure nothing changes. */
			[[nodiscard]] auto shrink(void) noexcept -> ca::result<void> requires ca::is_move_constructible<T> {

				if (_size == 0U) {
					this->_release();
					return ca::result<void>{};
				}
				const ca::usz count = ca::bit_ceil(_size);
				if (count == _capacity)
					return ca::result<void>{};
				auto block = ca::allocate_n<T>(_allocator, count);
				if (!block)
					return block.propagate();
				T* const into = static_cast<T*>(block->data());
				self::_relocate_runs(_data, _head, _size, _capacity, into);
				self::_free_block(_allocator, _data, _capacity);
				this->_adopt(*block, 0U, count);
				return ca::result<void>{};
			}

			/* @DOC Appends a T built from args at the back, growing when full. On failure nothing
			   changes. */
			template <typename... Args>
			requires ca::is_constructible<T, Args...> && ca::is_move_constructible<T>
			[[nodiscard]] auto emplace(Args&&... args) noexcept -> ca::result<void> {

				if (_size == _capacity) [[unlikely]] {
					T value(ca::forward<Args>(args)...);
					auto grown = this->_grow(this->_next_capacity());
					if (!grown)
						return grown;
					ca::construct_at(_data + this->_slot(_size), ca::move(value));
					++_size;
					return ca::result<void>{};
				}
				ca::construct_at(_data + this->_slot(_size), ca::forward<Args>(args)...);
				++_size;
				return ca::result<void>{};
			}

			/* @DOC Builds a T from args in front of the first element, growing when full. On
			   failure nothing changes. */
			template <typename... Args>
			requires ca::is_constructible<T, Args...> && ca::is_move_constructible<T>
			[[nodiscard]] auto emplace_front(Args&&... args) noexcept -> ca::result<void> {

				if (_size == _capacity) [[unlikely]] {
					T value(ca::forward<Args>(args)...);
					auto grown = this->_grow(this->_next_capacity());
					if (!grown)
						return grown;
					const ca::usz head = (_head - 1U) & (_capacity - 1U);
					ca::construct_at(_data + head, ca::move(value));
					_head = head;
					++_size;
					return ca::result<void>{};
				}
				const ca::usz head = (_head - 1U) & (_capacity - 1U);
				ca::construct_at(_data + head, ca::forward<Args>(args)...);
				_head = head;
				++_size;
				return ca::result<void>{};
			}

			/* @DOC emplace(value): deque.append. */
			[[nodiscard]] auto append(const T& value) noexcept -> ca::result<void> requires ca::is_copy_constructible<T> && ca::is_move_constructible<T> {
				return this->emplace(value);
			}

			[[nodiscard]] auto append(T&& value) noexcept -> ca::result<void> requires ca::is_move_constructible<T> {
				return this->emplace(ca::move(value));
			}

			/* @DOC emplace_front(value): deque.appendleft. */
			[[nodiscard]] auto prepend(const T& value) noexcept -> ca::result<void> requires ca::is_copy_constructible<T> && ca::is_move_constructible<T> {
				return this->emplace_front(value);
			}

			[[nodiscard]] auto prepend(T&& value) noexcept -> ca::result<void> requires ca::is_move_constructible<T> {
				return this->emplace_front(ca::move(value));
			}

			/* @DOC Copies the elements of view to the back, growing geometrically: deque.extend. view
			   must not be into this deque. On failure nothing changes. */
			[[nodiscard]] auto extend(const ca::borrowed_slice<const T> view) noexcept -> ca::result<void> requires ca::is_copy_constructible<T> && ca::is_move_constructible<T> {

				const ca::usz count = view.size();
				ca::usz total{0U};
				if (ca::add_overflow(total, _size, count)) [[unlikely]]
					return ca::failure{ca::errc::out_of_memory};
				if (total > _capacity) {
					if (total > ca::limits<ca::usz>::max() / 2U + 1U) [[unlikely]]
						return ca::failure{ca::errc::out_of_memory};
					auto grown = this->_grow(ca::max(ca::bit_ceil(total), this->_next_capacity()));
					if (!grown)
						return grown;
				}
				const ca::usz tail = this->_slot(_size);
				const ca::usz before_wrap = ca::min(count, _capacity - tail);
				ca::copy_construct_n(view.data(), before_wrap, _data + tail);
				ca::copy_construct_n(view.data() + before_wrap, count - before_wrap, _data);
				_size = total;
				return ca::result<void>{};
			}

			/* @DOC Removes and returns the back element, or nothing when empty: deque.pop. */
			[[nodiscard]] auto pop(void) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {

				if (_size == 0U)
					return ca::nullopt;
				--_size;
				T* const slot = _data + this->_slot(_size);
				ca::optional<T> last{ca::in_place, ca::move(*slot)};
				ca::destroy_at(slot);
				return last;
			}

			/* @DOC Removes and returns the front element, or nothing when empty: deque.popleft. */
			[[nodiscard]] auto pop_front(void) noexcept -> ca::optional<T> requires ca::is_move_constructible<T> {

				if (_size == 0U)
					return ca::nullopt;
				T* const slot = _data + _head;
				ca::optional<T> first{ca::in_place, ca::move(*slot)};
				ca::destroy_at(slot);
				_head = (_head + 1U) & (_capacity - 1U);
				--_size;
				return first;
			}

			/* @DOC Cleans up and destroys every element; keeps the capacity. Returns the first cleanup
			   error; every element is gone regardless. */
			[[nodiscard]] auto clear(void) noexcept -> ca::result<void> {

				ca::error first{};
				for (ca::usz i = 0U; i < _size; ++i) {
					T* const element = _data + this->_slot(i);
					if constexpr (ca::is_cleanable<T>) {
						element->cleanup().match([] {
						}, [&first](const ca::error& error) {
							if (first.ok())
								first = error;
						});
					}
					ca::destroy_at(element);
				}
				_head = 0U;
				_size = 0U;
				if (!first.ok())
					return ca::failure{first};
				return ca::result<void>{};
			}

			/* @DOC Cleans up every element (cleanup() when T has one), then frees the storage: empty,
			   no capacity. Returns the first cleanup error; everything is released regardless. */
			[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

				auto cleaned = this->clear();
				this->_release();
				return cleaned;
			}


			// -- methods ----------------------------------------------------

			/* @DOC The elements as two contiguous runs, front first; the second is empty unless the
			   elements wrap around the end of the ring. */
			[[nodiscard]] constexpr auto as_slices(void) noexcept -> ca::array<ca::borrowed_slice<T>, 2U> {

				const ca::usz front = ca::min(_size, _capacity - _head);
				return ca::array<ca::borrowed_slice<T>, 2U>{ca::borrowed_slice<T>{_data + _head, front}, ca::borrowed_slice<T>{_data, _size - front}};
			}

			[[nodiscard]] constexpr auto as_slices(void) const noexcept -> ca::array<ca::borrowed_slice<const T>, 2U> {

				const ca::usz front = ca::min(_size, _capacity - _head);
				return ca::array<ca::borrowed_slice<const T>, 2U>{ca::borrowed_slice<const T>{_data + _head, front}, ca::borrowed_slice<const T>{_data, _size - front}};
			}

			/* @DOC Index from the front of the first element equal to value, or nothing. */
			[[nodiscard]] constexpr auto find(const T& value) const noexcept -> ca::optional<ca::usz> {

				const auto runs = this->as_slices();
				const ca::optional<ca::usz> in_front = runs[0U].find(value);
				if (in_front)
					return *in_front;
				const ca::optional<ca::usz> in_back = runs[1U].find(value);
				if (in_back)
					return runs[0U].size() + *in_back;
				return ca::nullopt;
			}

			[[nodiscard]] constexpr auto contains(const T& value) const noexcept -> bool {
				return this->find(value).has_value();
			}

			/* @DOC Elements equal to value. */
			[[nodiscard]] constexpr auto count(const T& value) const noexcept -> ca::usz {

				const auto runs = this->as_slices();
				return runs[0U].count(value) + runs[1U].count(value);
			}

			/* @DOC A copy in memory from the same allocator, laid out from slot 0, capacity the power
			   of two at or above size(). Elements are copied with clone() when T has one, else with
			   the copy constructor. On a failed element clone, the partial copy is destroyed and that
			   error returned. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self> requires ca::is_clonable<T> || ca::is_copy_constructible<T> {
				return this->clone_in(_allocator);
			}

			/* @DOC clone, in memory from allocator. */
			[[nodiscard]] auto clone_in(const A& allocator) const noexcept -> ca::result<self> requires ca::is_clonable<T> || ca::is_copy_constructible<T> {

				self copy{allocator};
				if (_size == 0U)
					return copy;
				const ca::usz count = ca::bit_ceil(_size);
				auto block = ca::allocate_n<T>(allocator, count);
				if (!block)
					return block.propagate();
				copy._adopt(*block, 0U, count);
				if constexpr (ca::is_clonable<T>) {
					for (const T& element : *this) {
						auto cloned = element.clone();
						if (!cloned) {
							static_cast<void>(copy.cleanup());
							return cloned.propagate();
						}
						ca::construct_at(copy._data + copy._size, ca::move(*cloned));
						++copy._size;
					}
				}
				else {
					const auto runs = this->as_slices();
					ca::copy_construct_n(runs[0U].data(), runs[0U].size(), copy._data);
					ca::copy_construct_n(runs[1U].data(), runs[1U].size(), copy._data + runs[0U].size());
					copy._size = _size;
				}
				return copy;
			}


		private:

			// -- private methods --------------------------------------------

			[[nodiscard]] constexpr auto _slot(const ca::usz index) const noexcept -> ca::usz {
				return (_head + index) & (_capacity - 1U);
			}

			constexpr auto _expect_index(const ca::usz index) const noexcept -> void {
				if (index >= _size) [[unlikely]]
					ca::impl::index_out_of_bounds();
			}

			[[nodiscard]] auto _next_capacity(void) const noexcept -> ca::usz {

				if (_capacity == 0U)
					return self::_initial_capacity;
				if (_capacity > ca::limits<ca::usz>::max() / 2U) [[unlikely]]
					return 0U;
				return _capacity * 2U;
			}

			[[nodiscard]] static auto _block(T* const data, const ca::usz capacity) noexcept -> ca::allocation {
				return ca::allocation{data, capacity * sizeof(T)};
			}

			[[gnu::always_inline]] static auto _free_block(const A allocator, T* const data, const ca::usz capacity) noexcept -> void {
				ca::deallocate_n<T>(allocator, self::_block(data, capacity));
			}

			static auto _relocate_runs(T* const data, const ca::usz head, const ca::usz size, const ca::usz capacity, T* const into) noexcept -> void {

				const ca::usz front = ca::min(size, capacity - head);
				ca::relocate_n(data + head, front, into);
				ca::relocate_n(data, size - front, into + front);
			}

			[[nodiscard, gnu::noinline]] static auto _grown_block(const A allocator, T* const data, const ca::usz head, const ca::usz size, const ca::usz capacity, const ca::usz count) noexcept
			-> ca::result<ca::allocation> {

				if (count == 0U) [[unlikely]]
					return ca::failure{ca::errc::out_of_memory};
				if (data == nullptr)
					return ca::allocate_n<T>(allocator, count);
				const ca::usz front = ca::min(size, capacity - head);
				const ca::usz wrapped = size - front;
				const ca::usz moved_head = self::_grown_head(head, size, capacity, count);
				if constexpr (ca::is_trivially_relocatable<T>) {
					auto moved = ca::reallocate_n<T>(allocator, self::_block(data, capacity), count);
					if (!moved)
						return moved;
					T* const into = static_cast<T*>(moved->data());
					if (moved_head == head)
						ca::relocate_n(into, wrapped, into + capacity);
					else
						ca::relocate_n(into + head, front, into + moved_head);
					return moved;
				}
				else {
					auto expanded = ca::expand_n<T>(allocator, self::_block(data, capacity), count);
					if (expanded) {
						if (moved_head == head)
							ca::relocate_n(data, wrapped, data + capacity);
						else
							ca::relocate_n(data + head, front, data + moved_head);
						return *expanded;
					}
					auto moved = ca::allocate_n<T>(allocator, count);
					if (!moved)
						return moved.propagate();
					T* const into = static_cast<T*>(moved->data());
					ca::relocate_n(data + head, front, into + moved_head);
					ca::relocate_n(data, wrapped, into + (moved_head == head ? capacity : 0U));
					ca::deallocate_n<T>(allocator, self::_block(data, capacity));
					return moved;
				}
			}

			[[nodiscard]] static constexpr auto _grown_head(const ca::usz head, const ca::usz size, const ca::usz capacity, const ca::usz count) noexcept -> ca::usz {

				const ca::usz front = capacity - head;
				return size > front && size - front > front ? count - front : head;
			}

			auto _adopt(const ca::allocation block, const ca::usz head, const ca::usz capacity) noexcept -> void {

				    _data = static_cast<T*>(block.data());
				    _head = head;
				_capacity = capacity;
			}

			[[nodiscard, gnu::always_inline]] auto _grow(const ca::usz count) noexcept -> ca::result<void> {

				auto block = self::_grown_block(_allocator, _data, _head, _size, _capacity, count);
				if (!block)
					return block.propagate();
				this->_adopt(*block, _data == nullptr ? 0U : self::_grown_head(_head, _size, _capacity, count), count);
				return ca::result<void>{};
			}

			auto _release(void) noexcept -> void {

				if (_data == nullptr)
					return;
				for (ca::usz i = 0U; i < _size; ++i)
					ca::destroy_at(_data + this->_slot(i));
				self::_free_block(_allocator, _data, _capacity);
				    _data = nullptr;
				    _head = 0U;
				    _size = 0U;
				_capacity = 0U;
			}


			// -- assertions -------------------------------------------------

			static_assert(ca::is_object<T> && !ca::is_const<T> && !ca::is_array<T>,
				"ca::deque<T>: T must be a non-const, non-array object type");

	}; // class deque

} // namespace ca

#endif // ca_collections_array_deque_hpp
