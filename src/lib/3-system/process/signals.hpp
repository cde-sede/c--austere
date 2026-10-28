#ifndef ca_system_process_signals_hpp
#define ca_system_process_signals_hpp

#include "1-primitives/config/platform.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SIGNALS --------------------------------------------------------

		/* @DOC Signal numbers, without <signal.h>. Values are checked against it when the library
		   builds. resume is SIGCONT, terminal_stop SIGTSTP (the suspend key), background_read and
		   background_write SIGTTIN and SIGTTOU, window_resize SIGWINCH. */
		namespace signals {

			inline constexpr int hangup           = 1;
			inline constexpr int interrupt        = 2;
			inline constexpr int quit             = 3;
			inline constexpr int abort            = 6;
			inline constexpr int kill             = 9;
			inline constexpr int segv             = 11;
			inline constexpr int pipe             = 13;
			inline constexpr int alarm            = 14;
			inline constexpr int terminate        = 15;
			inline constexpr int background_read  = 21;
			inline constexpr int background_write = 22;
			inline constexpr int window_resize    = 28;

#if ca_platform(linux)
			inline constexpr int user1            = 10;
			inline constexpr int user2            = 12;
			inline constexpr int child            = 17;
			inline constexpr int resume           = 18;
			inline constexpr int stop             = 19;
			inline constexpr int terminal_stop    = 20;
#else
			inline constexpr int user1            = 30;
			inline constexpr int user2            = 31;
			inline constexpr int child            = 20;
			inline constexpr int resume           = 19;
			inline constexpr int stop             = 17;
			inline constexpr int terminal_stop    = 18;
#endif

		} // namespace signals

	} // namespace sys

} // namespace ca

#endif // ca_system_process_signals_hpp
