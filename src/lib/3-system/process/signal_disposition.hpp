#ifndef ca_system_process_signal_disposition_hpp
#define ca_system_process_signal_disposition_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SIGNAL DISPOSITION ---------------------------------------------

		/* @DOC What sigaction makes a signal do without a handler: its default action, or nothing. */
		enum class signal_disposition {
			default_action,
			ignore,
		};

	} // namespace sys

} // namespace ca

#endif // ca_system_process_signal_disposition_hpp
