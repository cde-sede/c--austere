#ifndef ca_system_io_get_terminal_size_hpp
#define ca_system_io_get_terminal_size_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/io/terminal_size.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- GET TERMINAL SIZE ----------------------------------------------

		/* @DOC Size of the terminal fd is attached to; fails on anything but a terminal. */
		[[nodiscard]] auto get_terminal_size(int fd) noexcept -> ca::result<ca::sys::terminal_size>;

	} // namespace sys

} // namespace ca

#endif // ca_system_io_get_terminal_size_hpp
