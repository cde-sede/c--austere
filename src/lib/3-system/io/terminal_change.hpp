#ifndef ca_system_io_terminal_change_hpp
#define ca_system_io_terminal_change_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TERMINAL CHANGE ------------------------------------------------

		/* @DOC When tcsetattr applies new settings: at once, once pending output is written, or then
		   also discarding unread input. */
		enum class terminal_change {
			now,
			drain,
			flush,
		};

	} // namespace sys

} // namespace ca

#endif // ca_system_io_terminal_change_hpp
