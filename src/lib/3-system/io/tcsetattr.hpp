#ifndef ca_system_io_tcsetattr_hpp
#define ca_system_io_tcsetattr_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/io/terminal_change.hpp"
#include "3-system/io/terminal_settings.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TCSETATTR ------------------------------------------------------

		/* @DOC Gives the terminal fd is attached to settings, applied when says; retries on EINTR. */
		[[nodiscard]] auto tcsetattr(int fd, const ca::sys::terminal_settings& settings,
			ca::sys::terminal_change when = ca::sys::terminal_change::now) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_io_tcsetattr_hpp
