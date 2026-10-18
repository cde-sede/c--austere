#ifndef ca_primitives_utility_forward_hpp
#define ca_primitives_utility_forward_hpp

#include "1-primitives/traits/is_lvalue_reference.hpp"
#include "1-primitives/traits/remove_reference.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FORWARD ------------------------------------------------------------

	/* @DOC Forwards value with the value category of T. */
	template <typename T>
	[[nodiscard]] constexpr auto forward(ca::remove_reference<T>& value) noexcept -> T&& {
		return static_cast<T&&>(value);
	}

	template <typename T>
	[[nodiscard]] constexpr auto forward(ca::remove_reference<T>&& value) noexcept -> T&& {

		static_assert(!ca::is_lvalue_reference<T>, "ca::forward: rvalue forwarded as an lvalue");
		return static_cast<T&&>(value);
	}

} // namespace ca

#endif // ca_primitives_utility_forward_hpp
