/* @DOC Performance of ca::slice<T, A>.

   Layout: three pointers (data, end, and the end of the block, which the allocator needs back),
   24 bytes on 64-bit, plus the allocator: nothing for a stateless one (ca::heap, the default).
   An empty slice owns no memory. The length is fixed at creation; there is no spare capacity
   to grow into, so nothing but the elements is ever constructed.

   | Operation | Time | Notes |
   |---|---|---|
   | operator[], get, first, last | O(1) | one compare |
   | size, empty, begin, end, data, view | O(1) | |
   | make(n, args...), from(view), clone | O(n) | one allocation; one memcpy for trivially copyable T in from and clone |
   | into_slice (dynamic_array) | O(1) | takes the array's block as is, no copy, no shrink |
   | find, count, contains, compare, == | O(n) | memchr/memcmp for code units, element loops otherwise |
   | find(other), find_last(other) | O(n * m) | |
   | fill, reverse | O(n) | |
   | destroy | O(n) | O(1) for trivially destructible T |

   Space: the block the allocator returns for n elements (the heap rounds to its size class),
   nothing else. A slice from dynamic_array::into_slice keeps the array's capacity.

   Locality and alignment: elements are contiguous in index order with stride sizeof(T), in a
   block aligned to alignof(T); scans vectorize. The header lives apart from the elements.

   Copy and move: the copy constructor is deleted; clone() is the explicit, fallible copy (clone()
   of each element when T has one). Move construction and assignment take three pointers and the
   allocator. TRIVIALLY_RELOCATABLE when A is: containers move slices with memcpy.

   Inlining: element access and the slice methods are inline; allocation goes through
   ca::allocate_n and the allocator's own fast path.

   Cleanup: destroy() runs each element's destroy() and destructor in one pass, then frees. */

#ifndef ca_collections_array_slice_hpp
#define ca_collections_array_slice_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/copy_construct_n.hpp"
#include "1-primitives/memory/destroy_n.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_object.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/impl/access.hpp"
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


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SLICE --------------------------------------------------------------

	/* @DOC Fixed-length run of T it owns, in memory from A: what a sequence becomes once its
	   length stops changing (Rust's Box<[T]>). Made by make(n, args...), from(view) or
	   dynamic_array::into_slice; those acquire memory and can fail. The sequence methods (cut,
	   find, compare, fill, reverse) come from ca::impl::slice_methods and return
	   borrowed_slices into it. Clean up with destroy(); the destructor is a failsafe that frees
	   silently. operator[] traps out of range; get allows the doubt. A stateful A has no default
	   construction and moves with the elements. */
	template <typename T, ca::is_allocator A = ca::heap>
	class slice final : public ca::impl::slice_methods<ca::slice<T, A>, T, const T> {


		private:

			// -- friends ----------------------------------------------------

			template <typename, ca::is_allocator>
			friend class dynamic_array; // qualify: ignore


			// -- private types ----------------------------------------------

			using self = ca::slice<T, A>;


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

			/* @DOC The same slice type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::slice<T, B>;


			// -- public constants -------------------------------------------

			/* @DOC Three pointers and the allocator, no self-reference: containers move a slice with
			   memcpy when A allows it. */
			static constexpr bool TRIVIALLY_RELOCATABLE = ca::is_trivially_relocatable<A>;


		private:

			// -- private members --------------------------------------------

			T*                      _data;
			T*                      _end;
			T*                      _limit;
			[[no_unique_address]] A _allocator;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr slice(void) noexcept requires ca::is_stateless_allocator<A>
			: _data{nullptr}, _end{nullptr}, _limit{nullptr}, _allocator{} {
			}

			/* @DOC Empty, owning nothing; what it is given comes from allocator. */
			constexpr explicit slice(const A& allocator) noexcept
			: _data{nullptr}, _end{nullptr}, _limit{nullptr}, _allocator{allocator} {
			}

			slice(const self&) = delete;

			constexpr slice(self&& other) noexcept
			: _data{ca::exchange(other._data, nullptr)}, _end{ca::exchange(other._end, nullptr)}, _limit{ca::exchange(other._limit, nullptr)},
			  _allocator{other._allocator} {
			}

			/* @DOC Failsafe for a skipped destroy(): destroys the elements and frees, silently. */
			~slice(void) noexcept {
				this->_release();
			}

			/* @DOC count elements, each built from args (value-initialized without), in memory from
			   allocator. */
			template <typename... Args>
			requires ca::is_constructible<T, const Args&...>
			[[nodiscard]] static auto make_in(const A& allocator, const ca::usz count, const Args&... args) noexcept -> ca::result<self> {

				self made{allocator};
				if (count == 0U)
					return made;
				auto block = ca::allocate_n<T>(allocator, count);
				if (!block)
					return block.propagate();
				made._adopt(*block, 0U);
				T* const data = made._data;
				for (ca::usz i = 0U; i < count; ++i)
					ca::construct_at(data + i, args...);
				made._end = data + count;
				return made;
			}

			template <typename... Args>
			requires ca::is_constructible<T, const Args&...> && ca::is_stateless_allocator<A>
			[[nodiscard]] static auto make(const ca::usz count, const Args&... args) noexcept -> ca::result<self> {
				return self::make_in(A{}, count, args...);
			}

			/* @DOC A copy of the elements of view, in memory from allocator: clone() of each when T
			   has one, else the copy constructor. On a failed element clone, the partial copy is
			   destroyed and that error returned. */
			[[nodiscard]] static auto from(const A& allocator, const ca::borrowed_slice<const T> view) noexcept -> ca::result<self>
			requires ca::is_clonable<T> || ca::is_copy_constructible<T> {

				self copy{allocator};
				const ca::usz size = view.size();
				if (size == 0U)
					return copy;
				auto block = ca::allocate_n<T>(allocator, size);
				if (!block)
					return block.propagate();
				copy._adopt(*block, 0U);
				if constexpr (ca::is_clonable<T>) {
					for (const T& element : view) {
						auto cloned = element.clone();
						if (!cloned) {
							static_cast<void>(copy.destroy());
							return cloned.propagate();
						}
						ca::construct_at(copy._end, ca::move(*cloned));
						++copy._end;
					}
				}
				else {
					ca::copy_construct_n(view.data(), size, copy._data);
					copy._end = copy._data + size;
				}
				return copy;
			}

			[[nodiscard]] static auto from(const ca::borrowed_slice<const T> view) noexcept -> ca::result<self>
			requires (ca::is_clonable<T> || ca::is_copy_constructible<T>) && ca::is_stateless_allocator<A> {
				return self::from(A{}, view);
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Takes other's contents and allocator; the previous contents are released as by the
			   destructor. Traps when T is cleanable and elements remain: destroy() first. */
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


			// -- methods ----------------------------------------------------

			/* @DOC The elements, as a view: what the shared methods work on. */
			[[nodiscard]] constexpr auto view(void) noexcept -> ca::borrowed_slice<T> {
				return ca::borrowed_slice<T>{_data, this->size()};
			}

			[[nodiscard]] constexpr auto view(void) const noexcept -> ca::borrowed_slice<const T> {
				return ca::borrowed_slice<const T>{_data, this->size()};
			}

			/* @DOC A copy in memory from the same allocator; see from. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self> requires ca::is_clonable<T> || ca::is_copy_constructible<T> {
				return self::from(_allocator, this->view());
			}

			/* @DOC clone, in memory from allocator. */
			[[nodiscard]] auto clone_in(const A& allocator) const noexcept -> ca::result<self> requires ca::is_clonable<T> || ca::is_copy_constructible<T> {
				return self::from(allocator, this->view());
			}

			/* @DOC Cleans up every element (destroy() when T has one), then frees the storage: empty.
			   Returns the first cleanup error; everything is released regardless. */
			[[nodiscard]] auto destroy(void) noexcept -> ca::result<void> {

				ca::error first{};
				if constexpr (ca::is_cleanable<T>) {
					for (T* element = _data; element != _end; ++element) {
						element->destroy().match([] {
						}, [&first](const ca::error& error) {
							if (first.ok())
								first = error;
						});
					}
				}
				this->_release();
				if (!first.ok())
					return ca::failure{first};
				return ca::result<void>{};
			}


		private:

			// -- private methods --------------------------------------------

			constexpr auto _expect_index(const ca::usz index) const noexcept -> void {
				if (index >= this->size()) [[unlikely]]
					ca::impl::index_out_of_bounds();
			}

			auto _adopt(const ca::allocation block, const ca::usz size) noexcept -> void {

				T* const data = static_cast<T*>(block.data());
				 _data = data;
				  _end = data + size;
				_limit = data + block.size() / sizeof(T);
			}

			auto _release(void) noexcept -> void {

				if (_data == nullptr)
					return;
				ca::destroy_n(_data, this->size());
				ca::deallocate_n<T>(_allocator, ca::allocation{_data, static_cast<ca::usz>(_limit - _data) * sizeof(T)});
				 _data = nullptr;
				  _end = nullptr;
				_limit = nullptr;
			}


			// -- assertions -------------------------------------------------

			static_assert(ca::is_object<T> && !ca::is_const<T> && !ca::is_array<T>,
				"ca::slice<T>: T must be a non-const, non-array object type");

	}; // class slice

} // namespace ca

#endif // ca_collections_array_slice_hpp
