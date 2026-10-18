/* @DOC The one std name braced lists need. The compiler turns `{1, 2, 3}` into a
   std::initializer_list<E> on its own, as a pointer and a length written straight into the
   object, so the class must be called that and have that layout (libstdc++'s and libc++'s).
   Defined here and nowhere else; ca code spells it ca::initializer_list. When a standard
   library's <initializer_list> came first, its definition is used; after this one, the
   libstdc++ and libc++ guards are set so theirs adds nothing. */

#ifndef ca_primitives_utility_initializer_list_hpp
#define ca_primitives_utility_initializer_list_hpp

#include "1-primitives/integers/usz.hpp"

#if !defined(_INITIALIZER_LIST) && !defined(_LIBCPP_INITIALIZER_LIST) // norm: ignore
#define _INITIALIZER_LIST // norm: ignore
#define _LIBCPP_INITIALIZER_LIST // norm: ignore


// -- STD NAMESPACE ----------------------------------------------------------

namespace std { // norm: ignore


	// -- INITIALIZER LIST ---------------------------------------------------

	/* @DOC Read-only view of the array behind a braced list; lives as long as the full
	   expression that made it (or the variable it initializes). Built by the compiler only. */
	template <typename E>
	class initializer_list final { // norm: ignore


		private:

			// -- private types ----------------------------------------------

			using self = std::initializer_list<E>; // norm: ignore


		public:

			// -- public types -----------------------------------------------

			using value_type      = E;
			using reference       = const E&;
			using const_reference = const E&;
			using size_type       = ca::usz;
			using iterator        = const E*;
			using const_iterator  = const E*;


		private:

			// -- private members --------------------------------------------

			const E* _array;
			ca::usz  _size;

			constexpr initializer_list(const E* const array, const ca::usz size) noexcept
			: _array{array}, _size{size} {
			}


		public:

			// -- public lifecycle -------------------------------------------

			constexpr initializer_list(void) noexcept
			: _array{nullptr}, _size{0U} {
			}


			// -- iterators --------------------------------------------------

			[[nodiscard]] constexpr auto begin(void) const noexcept -> const E* {
				return _array;
			}

			[[nodiscard]] constexpr auto end(void) const noexcept -> const E* {
				return _array + _size;
			}


			// -- accessors --------------------------------------------------

			/* @DOC Number of elements. */
			[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
				return _size;
			}

			/* @DOC The elements; ca::is_contiguous, so it converts to a borrowed_slice<const E>. */
			[[nodiscard]] constexpr auto data(void) const noexcept -> const E* {
				return _array;
			}

	}; // class initializer_list

} // namespace std

#endif


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- INITIALIZER LIST ---------------------------------------------------

	/* @DOC std::initializer_list<E>: what `{a, b, c}` becomes in a range-for or as an argument. */
	template <typename E>
	using initializer_list = std::initializer_list<E>; // no-std: ignore

} // namespace ca

#endif // ca_primitives_utility_initializer_list_hpp
