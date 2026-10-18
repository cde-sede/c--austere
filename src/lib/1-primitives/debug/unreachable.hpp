#ifndef ca_primitives_debug_unreachable_hpp
#define ca_primitives_debug_unreachable_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNREACHABLE --------------------------------------------------------

	/* @DOC Marks a point as unreachable; reaching it is undefined. */
	[[noreturn]] inline auto unreachable(void) noexcept -> void {
		__builtin_unreachable();
	}

} // namespace ca

#endif // ca_primitives_debug_unreachable_hpp
