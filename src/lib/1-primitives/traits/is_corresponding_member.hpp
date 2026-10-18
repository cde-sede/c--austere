#ifndef ca_primitives_traits_is_corresponding_member_hpp
#define ca_primitives_traits_is_corresponding_member_hpp

#include "1-primitives/config/has_builtin.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS CORRESPONDING MEMBER --------------------------------------------

	/* @DOC std::is_corresponding_member. Needs __builtin_is_corresponding_member; fails to compile with
	   a ca: message without it (clang 18). */
#if ca_has_builtin(__builtin_is_corresponding_member)
	template <typename S1, typename S2, typename M1, typename M2>
	constexpr auto is_corresponding_member(M1 S1::* first, M2 S2::* second) noexcept -> bool {
		return __builtin_is_corresponding_member(first, second);
	}
#else
	template <typename S1, typename S2, typename M1, typename M2>
	constexpr auto is_corresponding_member(M1 S1::*, M2 S2::*) noexcept -> bool {

		static_assert(false, "ca: is_corresponding_member needs __builtin_is_corresponding_member");
		return false;
	}
#endif

} // namespace ca

#endif // ca_primitives_traits_is_corresponding_member_hpp
