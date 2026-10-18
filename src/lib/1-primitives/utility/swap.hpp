#ifndef ca_primitives_utility_swap_hpp
#define ca_primitives_utility_swap_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_move_assignable.hpp"
#include "1-primitives/traits/is_move_constructible.hpp"
#include "1-primitives/traits/is_nothrow_move_assignable.hpp"
#include "1-primitives/traits/is_nothrow_move_constructible.hpp"
#include "1-primitives/traits/is_nothrow_swappable.hpp"
#include "1-primitives/traits/is_swappable.hpp"
#include "1-primitives/utility/move.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SWAP ---------------------------------------------------------------

	/* @DOC Exchanges a and b by move. Arrays: element by element, through ADL swap. */
	template <typename T>
	requires ca::is_move_constructible<T> && ca::is_move_assignable<T>
	constexpr auto swap(T& a, T& b) noexcept(ca::is_nothrow_move_constructible<T> && ca::is_nothrow_move_assignable<T>) -> void {

		T tmp = ca::move(a);
		a = ca::move(b);
		b = ca::move(tmp);
	}

	template <typename T, ca::usz N>
	requires ca::is_swappable<T>
	constexpr auto swap(T (&a)[N], T (&b)[N]) noexcept(ca::is_nothrow_swappable<T>) -> void {
		for (ca::usz i = 0U; i < N; ++i)
			swap(a[i], b[i]); // qualify: ignore
	}

} // namespace ca

#endif // ca_primitives_utility_swap_hpp
