#ifndef ca_system_io_poll_events_hpp
#define ca_system_io_poll_events_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- POLL EVENTS ----------------------------------------------------

		/* @DOC poll event bits, without <poll.h>. Checked against it when the library builds. The
		   last three are only ever reported, never requested. */
		namespace poll_events {

			inline constexpr short readable = 0x1;
			inline constexpr short urgent   = 0x2;
			inline constexpr short writable = 0x4;
			inline constexpr short error    = 0x8;
			inline constexpr short hangup   = 0x10;
			inline constexpr short invalid  = 0x20;

		} // namespace poll_events

	} // namespace sys

} // namespace ca

#endif // ca_system_io_poll_events_hpp
