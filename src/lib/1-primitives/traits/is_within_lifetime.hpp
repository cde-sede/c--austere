#ifndef ca_primitives_traits_is_within_lifetime_hpp
#define ca_primitives_traits_is_within_lifetime_hpp

#include "1-primitives/config/has_builtin.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS WITHIN LIFETIME -------------------------------------------------

	/* @DOC std::is_within_lifetime. Needs __builtin_is_within_lifetime; fails to compile with a ca:
	   message without it. */
#if ca_has_builtin(__builtin_is_within_lifetime)
	template <typename T>
	consteval auto is_within_lifetime(const T* pointer) noexcept -> bool {
		return __builtin_is_within_lifetime(pointer);
	}
#else
	template <typename T>
	consteval auto is_within_lifetime(const T*) noexcept -> bool {

		static_assert(false, "ca: is_within_lifetime needs __builtin_is_within_lifetime");
		return false;
	}
#endif

} // namespace ca

#endif // ca_primitives_traits_is_within_lifetime_hpp
