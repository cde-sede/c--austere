#ifndef ca_primitives_utility_exchange_hpp
#define ca_primitives_utility_exchange_hpp

#include "1-primitives/traits/is_nothrow_assignable.hpp"
#include "1-primitives/traits/is_nothrow_move_constructible.hpp"
#include "1-primitives/utility/forward.hpp"
#include "1-primitives/utility/move.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- EXCHANGE -----------------------------------------------------------

	/* @DOC Replaces object with value; returns the old value. */
	template <typename T, typename U = T>
	constexpr auto exchange(T& object, U&& value) noexcept(ca::is_nothrow_move_constructible<T> && ca::is_nothrow_assignable<T&, U>) -> T {

		T old = ca::move(object);
		object = ca::forward<U>(value);
		return old;
	}

} // namespace ca

#endif // ca_primitives_utility_exchange_hpp
