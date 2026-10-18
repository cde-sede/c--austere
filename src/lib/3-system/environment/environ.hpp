#ifndef ca_system_environment_environ_hpp
#define ca_system_environment_environ_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- ENVIRON --------------------------------------------------------

		/* @DOC The environment as a null-terminated array of "NAME=value", as execve takes it. Valid until
		   the environment changes. Never fails. */
		[[nodiscard]] auto environ(void) noexcept -> char* const*;

	} // namespace sys

} // namespace ca

#endif // ca_system_environment_environ_hpp
