#ifndef ca_primitives_debug_trap_hpp
#define ca_primitives_debug_trap_hpp

#include "1-primitives/debug/recovery.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRAP ---------------------------------------------------------------

	/* @DOC Leaves for the innermost context that takes it (ca::context_manager, ca::with), which
	   fails with reason; the contexts it passes run their leave on the way. With no context
	   taking it, stops the program with an illegal instruction, right here. reason must outlive
	   the context: a literal. Frames between the trap and the context are skipped, their objects
	   never destroyed. */
	[[noreturn]] inline auto trap(const char* const reason = nullptr) noexcept -> void {

		if (ca::impl::current_recovery != nullptr) [[unlikely]]
			ca::impl::stop(reason);
		__builtin_trap();
	}

} // namespace ca

#endif // ca_primitives_debug_trap_hpp
