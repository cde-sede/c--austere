#ifndef ca_primitives_debug_trap_hpp
#define ca_primitives_debug_trap_hpp

#include "1-primitives/debug/recovery.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRAP ---------------------------------------------------------------

	/* @DOC Leaves for the thread's innermost context (ca::context_manager, ca::with), which fails
	   with reason; outside every context, stops the program with an illegal instruction. reason
	   must outlive the context: a literal. Frames between the trap and the context are skipped,
	   their objects never destroyed. */
	[[noreturn]] inline auto trap(const char* const reason = nullptr) noexcept -> void {

		ca::impl::recovery* const point = ca::impl::current_recovery;
		if (point != nullptr) [[unlikely]] {
			point->_reason = reason;
			ca::impl::recover(point);
		}
		__builtin_trap();
	}

} // namespace ca

#endif // ca_primitives_debug_trap_hpp
