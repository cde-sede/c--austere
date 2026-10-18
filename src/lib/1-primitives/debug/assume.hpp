#ifndef ca_primitives_debug_assume_hpp
#define ca_primitives_debug_assume_hpp

#include "1-primitives/config/has_builtin.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ASSUME -------------------------------------------------------------

	/* @DOC Lets the optimizer assume condition; undefined if false. condition is evaluated. */
	constexpr auto assume(const bool condition) noexcept -> void {

#if ca_has_builtin(__builtin_assume)
		__builtin_assume(condition);
#else
		if (!condition)
			__builtin_unreachable();
#endif
	}

} // namespace ca

#endif // ca_primitives_debug_assume_hpp
