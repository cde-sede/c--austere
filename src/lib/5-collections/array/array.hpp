/* @DOC Performance of ca::array<T, N>.

   Layout: N elements in the object itself, nothing else: sizeof(array<T, N>) is N * sizeof(T),
   alignment alignof(T). No pointer, no size, no allocation; array<T, 0> is empty.

   | Operation | Time | Notes |
   |---|---|---|
   | operator[], get, first, last | O(1) | one compare against the constant N; folds away for constant indices |
   | size, empty, begin, end, data, view | O(1) | size() is a constant expression |
   | construction | O(N) | value-initialized, or from N values; constexpr |
   | find, count, contains, compare, == | O(N) | memchr/memcmp for code units, element loops otherwise |
   | fill, reverse | O(N) | |
   | cleanup, clone | O(N) | only for cleanable and clonable T |

   Space: exactly the elements. Locality and alignment: contiguous, stride sizeof(T), wherever
   the array lives (stack, inside another object, static storage); scans vectorize and the
   bounds are known to the compiler.

   Copy and move: those of T, element by element; trivially copyable when T is, then copied as
   bytes and usable in constant expressions. TRIVIALLY_RELOCATABLE when T is.

   Inlining: every member is a constexpr inline one-liner over the built-in array.

   Against std::array: same layout and code; operator[] checks its index in every build (std
   only with assertions enabled), the check disappearing when the index is a constant or
   bounded by size(). */

#ifndef ca_collections_array_array_hpp
#define ca_collections_array_array_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_default_constructible.hpp"
#include "1-primitives/traits/is_move_assignable.hpp"
#include "1-primitives/traits/is_object.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/utility/forward.hpp"
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


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ARRAY --------------------------------------------------------------

	namespace impl {

		/* @DOC The elements of an array<T, N>; none for N = 0, where T[0] cannot be formed. */
		template <typename T, ca::usz N>
		struct array_storage final {

				using self = ca::impl::array_storage<T, N>;

				T _values[N];

		}; // struct array_storage

		template <typename T>
		struct array_storage<T, 0U> final {

				using self = ca::impl::array_storage<T, 0U>;

		}; // struct array_storage

	} // namespace impl


	/* @DOC N elements of T held in the object itself: std::array, with the sequence methods
	   (cut, find, compare, fill, reverse) of ca::impl::slice_methods returning borrowed_slices
	   into it. Built value-initialized or from exactly N values (`ca::array a{1, 2, 3}` deduces
	   array<int, 3>); copies, moves and destroys as T does; constexpr. operator[] traps out of
	   range; get allows the doubt. cleanup() and clone() for cleanable and clonable T. */
	template <typename T, ca::usz N>
	class array final : public ca::impl::slice_methods<ca::array<T, N>, T, const T> {


		private:

			// -- private types ----------------------------------------------

			using self = ca::array<T, N>;


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

			/* @DOC Number of elements. */
			static constexpr ca::usz SIZE = N;

			/* @DOC No pointer into itself: containers move an array with memcpy when T allows it. */
			static constexpr bool TRIVIALLY_RELOCATABLE = ca::is_trivially_relocatable<T>;


		private:

			// -- private members --------------------------------------------

			ca::impl::array_storage<T, N> _storage;


		public:

			// -- public lifecycle -------------------------------------------

			/* @DOC Every element value-initialized: zero for scalars. */
			constexpr array(void) noexcept requires (N == 0U || ca::is_default_constructible<T>)
			: _storage{} {
			}

			/* @DOC The elements from values, in order; exactly N of them. */
			template <typename... U>
			requires (N != 0U && sizeof...(U) == N && (... && ca::is_convertible<U, T>))
			constexpr array(U&&... values) noexcept
			: _storage{{T(ca::forward<U>(values))...}} {
			}


			// -- iterators --------------------------------------------------

			[[nodiscard]] constexpr auto begin(void) noexcept -> T* {
				return this->data();
			}

			[[nodiscard]] constexpr auto begin(void) const noexcept -> const T* {
				return this->data();
			}

			[[nodiscard]] constexpr auto end(void) noexcept -> T* {
				return this->data() + N;
			}

			[[nodiscard]] constexpr auto end(void) const noexcept -> const T* {
				return this->data() + N;
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] static constexpr auto size(void) noexcept -> ca::usz {
				return N;
			}

			[[nodiscard]] static constexpr auto empty(void) noexcept -> bool {
				return N == 0U;
			}

			/* @DOC The first element; nullptr when N is 0. */
			[[nodiscard]] constexpr auto data(void) noexcept -> T* {

				if constexpr (N == 0U)
					return nullptr;
				else
					return _storage._values;
			}

			[[nodiscard]] constexpr auto data(void) const noexcept -> const T* {

				if constexpr (N == 0U)
					return nullptr;
				else
					return _storage._values;
			}


			// -- element access ---------------------------------------------

			/* @DOC Element at index. Traps when index >= N, in every build. */
			[[nodiscard]] constexpr auto operator[](const ca::usz index) noexcept -> T& {

				self::_expect_index(index);
				return this->data()[index];
			}

			[[nodiscard]] constexpr auto operator[](const ca::usz index) const noexcept -> const T& {

				self::_expect_index(index);
				return this->data()[index];
			}

			/* @DOC Element at index, or nothing when index >= N. */
			[[nodiscard]] constexpr auto get(const ca::usz index) noexcept -> ca::optional<T&> {

				if (index >= N)
					return ca::nullopt;
				return ca::optional<T&>{this->data()[index]};
			}

			[[nodiscard]] constexpr auto get(const ca::usz index) const noexcept -> ca::optional<const T&> {

				if (index >= N)
					return ca::nullopt;
				return ca::optional<const T&>{this->data()[index]};
			}


			// -- methods ----------------------------------------------------

			/* @DOC The elements, as a view: what the shared methods work on. */
			[[nodiscard]] constexpr auto view(void) noexcept -> ca::borrowed_slice<T> {
				return ca::borrowed_slice<T>{this->data(), N};
			}

			[[nodiscard]] constexpr auto view(void) const noexcept -> ca::borrowed_slice<const T> {
				return ca::borrowed_slice<const T>{this->data(), N};
			}

			/* @DOC A copy: clone() of each element, else the copy constructor. A clonable T must be
			   default-constructible and move-assignable, the clones being assigned in place; on a
			   failed clone, the clones made so far are cleaned up and that error returned. */
			[[nodiscard]] constexpr auto clone(void) const noexcept -> ca::result<self>
			requires (ca::is_clonable<T> && ca::is_default_constructible<T> && ca::is_move_assignable<T>) || (!ca::is_clonable<T> && ca::is_copy_constructible<T>) {

				if constexpr (ca::is_clonable<T>) {
					self copy{};
					for (ca::usz i = 0U; i < N; ++i) {
						auto cloned = this->data()[i].clone();
						if (!cloned) {
							if constexpr (ca::is_cleanable<T>) {
								for (ca::usz j = 0U; j < i; ++j)
									static_cast<void>(copy.data()[j].cleanup());
							}
							return cloned.propagate();
						}
						copy.data()[i] = ca::move(*cloned);
					}
					return copy;
				}
				else
					return self{*this};
			}

			/* @DOC Cleans up every element, in order; they stay in place, ready for their
			   destructors. Returns the first cleanup error; every element is cleaned up regardless. */
			[[nodiscard]] constexpr auto cleanup(void) noexcept -> ca::result<void> requires ca::is_cleanable<T> {

				ca::error first{};
				for (T& element : *this) {
					element.cleanup().match([] {
					}, [&first](const ca::error& error) {
						if (first.ok())
							first = error;
					});
				}
				if (!first.ok())
					return ca::failure{first};
				return ca::result<void>{};
			}


		private:

			// -- private methods --------------------------------------------

			static constexpr auto _expect_index(const ca::usz index) noexcept -> void {
				if (index >= N) [[unlikely]]
					ca::impl::index_out_of_bounds();
			}


			// -- assertions -------------------------------------------------

			static_assert(ca::is_object<T> && !ca::is_const<T> && !ca::is_array<T>,
				"ca::array<T, N>: T must be a non-const, non-array object type");

	}; // class array


	template <typename T, typename... U>
	requires (... && ca::is_same<T, U>)
	array(T, U...) -> array<T, 1U + sizeof...(U)>; // qualify: ignore

} // namespace ca

#endif // ca_collections_array_array_hpp
