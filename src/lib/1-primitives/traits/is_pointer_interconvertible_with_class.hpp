#ifndef ca_primitives_traits_is_pointer_interconvertible_with_class_hpp
#define ca_primitives_traits_is_pointer_interconvertible_with_class_hpp

#include "1-primitives/config/has_builtin.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS POINTER INTERCONVERTIBLE WITH CLASS -----------------------------

	/* @DOC std::is_pointer_interconvertible_with_class. Needs
	   __builtin_is_pointer_interconvertible_with_class; fails to compile with a ca: message without it
	   (clang 18). */
#if ca_has_builtin(__builtin_is_pointer_interconvertible_with_class)
	template <typename S, typename M>
	constexpr auto is_pointer_interconvertible_with_class(M S::* member) noexcept -> bool {
		return __builtin_is_pointer_interconvertible_with_class(member);
	}
#else
	template <typename S, typename M>
	constexpr auto is_pointer_interconvertible_with_class(M S::*) noexcept -> bool {

		static_assert(false, "ca: is_pointer_interconvertible_with_class needs __builtin_is_pointer_interconvertible_with_class");
		return false;
	}
#endif

} // namespace ca

#endif // ca_primitives_traits_is_pointer_interconvertible_with_class_hpp
