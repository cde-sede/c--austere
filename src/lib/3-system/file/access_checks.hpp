#ifndef ca_system_file_access_checks_hpp
#define ca_system_file_access_checks_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- ACCESS CHECKS --------------------------------------------------

		/* @DOC What access and faccessat test, without <unistd.h>: exists, or any of read, write and
		   execute. Checked against it when the library builds. */
		namespace access_checks {

			inline constexpr int exists  = 0;
			inline constexpr int execute = 1;
			inline constexpr int write   = 2;
			inline constexpr int read    = 4;

		} // namespace access_checks

	} // namespace sys

} // namespace ca

#endif // ca_system_file_access_checks_hpp
