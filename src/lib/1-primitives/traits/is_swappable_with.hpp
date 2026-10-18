#ifndef ca_primitives_traits_is_swappable_with_hpp
#define ca_primitives_traits_is_swappable_with_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_lvalue_reference.hpp"
#include "1-primitives/traits/is_move_assignable.hpp"
#include "1-primitives/traits/is_move_constructible.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_reference.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS SWAPPABLE WITH --------------------------------------------------

	namespace impl {

		template <typename T, ca::usz N>
		using array_ref = T (&)[N];

		template <typename T, typename U>
		concept is_adl_swappable = requires(T&& t, U&& u) {
			swap(static_cast<T&&>(t), static_cast<U&&>(u)); // qualify: ignore
		};

		template <typename T, typename U>
		inline constexpr bool swappable_with = ca::impl::is_adl_swappable<T, U>
			|| (ca::is_lvalue_reference<T> && ca::is_same<T, U>
			&& ca::is_move_constructible<ca::remove_reference<T>>
			&& ca::is_move_assignable<ca::remove_reference<T>>);

		template <typename T, ca::usz N>
		inline constexpr bool swappable_with<ca::impl::array_ref<T, N>, ca::impl::array_ref<T, N>> = ca::impl::is_adl_swappable<ca::impl::array_ref<T, N>, ca::impl::array_ref<T, N>>
			|| ca::impl::swappable_with<T&, T&>;

	} // namespace impl


	/* @DOC std::is_swappable_with_v, as a concept. */
	template <typename T, typename U>
	concept is_swappable_with = ca::impl::swappable_with<T, U> && ca::impl::swappable_with<U, T>;

} // namespace ca

#endif // ca_primitives_traits_is_swappable_with_hpp
