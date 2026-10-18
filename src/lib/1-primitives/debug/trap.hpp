#ifndef ca_primitives_debug_trap_hpp
#define ca_primitives_debug_trap_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRAP ---------------------------------------------------------------

	/* @DOC Stops the program with an illegal instruction. */
	[[noreturn]] inline auto trap(void) noexcept -> void {
		__builtin_trap();
	}

} // namespace ca

#endif // ca_primitives_debug_trap_hpp
