#ifndef ca_system_process_exit_immediately_hpp
#define ca_system_process_exit_immediately_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- EXIT IMMEDIATELY -----------------------------------------------

		/* @DOC Ends the process with status, running no handler and flushing nothing: what a forked
		   child calls when exec fails. */
		[[noreturn]] auto exit_immediately(int status) noexcept -> void;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_exit_immediately_hpp
