#ifndef ca_system_process_usage_target_hpp
#define ca_system_process_usage_target_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- USAGE TARGET ---------------------------------------------------

		/* @DOC Whose usage getrusage reports: the process, or its waited-for children. */
		enum class usage_target {
			process,
			children,
		};

	} // namespace sys

} // namespace ca

#endif // ca_system_process_usage_target_hpp
