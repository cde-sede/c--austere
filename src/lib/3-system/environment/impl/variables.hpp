#ifndef ca_system_environment_impl_variables_hpp
#define ca_system_environment_impl_variables_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- VARIABLES ------------------------------------------------------

		/* @DOC The environment without libc, on the C environ array (the kernel's envp, which
		   libca_runtime's _start points it at): what libca_runtime's getenv, setenv and unsetenv
		   are. In libc mode ca::sys::getenv and the others stay libc's, which owns environ there.
		   Linux x86-64 only. Not thread-safe, as glibc's. */
		namespace impl {

			/* @DOC The value of name, or null when unset. */
			[[nodiscard]] auto find_variable(const char* name) noexcept -> char*;

			/* @DOC Sets name to value, an existing one kept unless overwrite: "NAME=value" copied into
			   pages of its own and, to add a name, the array into a larger one; what it replaces
			   stays allocated, as with glibc. errc::invalid_argument for an empty name or one with
			   '=', errc::out_of_memory. */
			[[nodiscard]] auto set_variable(const char* name, const char* value, bool overwrite) noexcept -> ca::result<void>;

			/* @DOC Removes every entry of name. errc::invalid_argument as set_variable. */
			[[nodiscard]] auto unset_variable(const char* name) noexcept -> ca::result<void>;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_environment_impl_variables_hpp
