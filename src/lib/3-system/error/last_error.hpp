#ifndef ca_system_error_last_error_hpp
#define ca_system_error_last_error_hpp

#include "2-vocabulary/error.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	/* @DOC The system call layer: thin wrappers over libc and POSIX, the only code that
	   touches them. Fallible calls return result; system headers never reach a public header,
	   so no libc or std name leaks. */
	namespace sys {


		// -- LAST ERROR -----------------------------------------------------

		namespace impl {

			extern constinit thread_local int last_code;

			/* @DOC Reads libc's errno, records it as the last error and returns it. Only for the
			   libc fallback paths, in its own object so that code without them never links
			   errno. */
			[[nodiscard]] auto libc_error(void) noexcept -> ca::error;

		} // namespace impl

		/* @DOC The calling thread's last system error: ca's own errno, set by every failing
		   ca::syscall call and libc fallback, never by libc itself. A plain thread-local
		   access, no call. */
		[[nodiscard]] inline auto last_error(void) noexcept -> ca::error {
			return ca::error{ca::sys::impl::last_code};
		}

		inline auto set_last_error(const ca::error error) noexcept -> void {
			ca::sys::impl::last_code = error.code();
		}

	} // namespace sys

} // namespace ca

#endif // ca_system_error_last_error_hpp
