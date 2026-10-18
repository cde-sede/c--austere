#ifndef ca_vocabulary_realized_hpp
#define ca_vocabulary_realized_hpp

#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/is_reference.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/reference_constructs_from_temporary.hpp"
#include "1-primitives/traits/is_void.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/utility/addressof.hpp"
#include "1-primitives/utility/forward.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/in_place_t.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REALIZED -----------------------------------------------------------

	namespace impl {

		template <typename T, typename... Args>
		concept is_holdable = ca::is_constructible<T, Args...>
			&& (!ca::is_reference<T> || (sizeof...(Args) == 1U && (... && !ca::reference_constructs_from_temporary<T, Args>)));

	} // namespace impl


	/* @DOC Always holds a T: what a checked uncertain container is. realized<T&> refers to one. */
	template <typename T>
	class realized final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::realized<T>;


		public:

			// -- public types -----------------------------------------------

			using value_type = T;


		private:

			// -- private members --------------------------------------------

			T _value;


		public:

			// -- public lifecycle -------------------------------------------

			template <typename U = T>
			requires (!ca::is_same<ca::remove_cvref<U>, ca::realized<T>> && !ca::is_same<ca::remove_cvref<U>, ca::in_place_t>
				&& ca::is_constructible<T, U>)
			constexpr explicit(!ca::is_convertible<U, T>) realized(U&& value) noexcept
			: _value(ca::forward<U>(value)) { // init-list: ignore
			}

			template <typename... Args>
			requires ca::is_constructible<T, Args...>
			constexpr explicit realized(ca::in_place_t, Args&&... args) noexcept
			: _value(ca::forward<Args>(args)...) { // init-list: ignore
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto operator->(void) noexcept -> T* {
				return ca::addressof(_value);
			}

			[[nodiscard]] constexpr auto operator->(void) const noexcept -> const T* {
				return ca::addressof(_value);
			}

			[[nodiscard]] constexpr auto operator*(void) & noexcept -> T& {
				return _value;
			}

			[[nodiscard]] constexpr auto operator*(void) const& noexcept -> const T& {
				return _value;
			}

			[[nodiscard]] constexpr auto operator*(void) && noexcept -> T&& {
				return ca::move(_value);
			}


			// -- assertions -------------------------------------------------

			static_assert(!ca::is_void<T> && !ca::is_reference<T>, "ca::realized<T>: T must be a non-void, non-reference type");

	}; // class realized


	/* @DOC Refers to a T; never binds a temporary. Assignment rebinds. */
	template <typename T>
	class realized<T&> final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::realized<T&>;


		public:

			// -- public types -----------------------------------------------

			using value_type = T&;


		private:

			// -- private members --------------------------------------------

			T* _pointer;


		public:

			// -- public lifecycle -------------------------------------------

			template <typename U>
			requires (!ca::is_same<ca::remove_cvref<U>, ca::realized<T&>> && !ca::is_same<ca::remove_cvref<U>, ca::in_place_t>
				&& ca::impl::is_holdable<T&, U>)
			constexpr explicit(!ca::is_convertible<U, T&>) realized(U&& value) noexcept
			: _pointer{ca::addressof(static_cast<T&>(ca::forward<U>(value)))} {
			}

			template <typename U>
			requires ca::impl::is_holdable<T&, U>
			constexpr explicit realized(ca::in_place_t, U&& value) noexcept
			: _pointer{ca::addressof(static_cast<T&>(ca::forward<U>(value)))} {
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto operator->(void) const noexcept -> T* {
				return _pointer;
			}

			[[nodiscard]] constexpr auto operator*(void) const noexcept -> T& {
				return *_pointer;
			}

	}; // class realized


	template <typename T>
	realized(T) -> realized<T>; // qualify: ignore

} // namespace ca

#endif // ca_vocabulary_realized_hpp
