#ifndef ca_system_io_poll_hpp
#define ca_system_io_poll_hpp

#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/io/poll_entry.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- POLL -----------------------------------------------------------

		/* @DOC Waits until an entry's events occur or timeout_ms passes (-1: forever, 0: no wait);
		   returns how many entries report events. Interrupted by a signal: errc::interrupted,
		   left to the caller, who knows the remaining time. */
		[[nodiscard]] auto poll(ca::span<ca::sys::poll_entry> entries, ca::i32 timeout_ms) noexcept -> ca::result<ca::usz>;

	} // namespace sys

} // namespace ca

#endif // ca_system_io_poll_hpp
