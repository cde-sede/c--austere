#ifndef ca_system_io_enter_raw_mode_hpp
#define ca_system_io_enter_raw_mode_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/io/terminal_settings.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- ENTER RAW MODE -------------------------------------------------

		/* @DOC Puts the terminal fd is attached to in raw mode (terminal_settings::raw), discarding
		   unread input; returns the settings it had, for tcsetattr to restore. */
		[[nodiscard]] auto enter_raw_mode(int fd) noexcept -> ca::result<ca::sys::terminal_settings>;

	} // namespace sys

} // namespace ca

#endif // ca_system_io_enter_raw_mode_hpp
