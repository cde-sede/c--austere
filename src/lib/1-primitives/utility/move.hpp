#ifndef ca_primitives_utility_move_hpp
#define ca_primitives_utility_move_hpp

#include "1-primitives/traits/remove_reference.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MOVE ---------------------------------------------------------------

	/* @DOC Casts to an rvalue: the callee may take ownership. */
	template <typename T>
	[[nodiscard]] constexpr auto move(T&& value) noexcept -> ca::remove_reference<T>&& {
		return static_cast<ca::remove_reference<T>&&>(value);
	}

} // namespace ca

#endif // ca_primitives_utility_move_hpp
