#ifndef ca_system_process_exit_hpp
#define ca_system_process_exit_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- EXIT -----------------------------------------------------------

		/* @DOC Ends the process with status after atexit handlers and stdio flushing. */
		[[noreturn]] auto exit(int status) noexcept -> void;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_exit_hpp
