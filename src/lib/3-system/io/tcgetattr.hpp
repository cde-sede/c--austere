#ifndef ca_system_io_tcgetattr_hpp
#define ca_system_io_tcgetattr_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/io/terminal_settings.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TCGETATTR ------------------------------------------------------

		/* @DOC Settings of the terminal fd is attached to; fails with not_a_terminal on anything else. */
		[[nodiscard]] auto tcgetattr(int fd) noexcept -> ca::result<ca::sys::terminal_settings>;

	} // namespace sys

} // namespace ca

#endif // ca_system_io_tcgetattr_hpp
