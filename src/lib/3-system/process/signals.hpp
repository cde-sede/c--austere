#ifndef ca_system_process_signals_hpp
#define ca_system_process_signals_hpp

#include "1-primitives/config/platform.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SIGNALS --------------------------------------------------------

		/* @DOC Signal numbers, without <signal.h>. Values are checked against it when the library
		   builds. */
		namespace signals {

			inline constexpr int hangup    = 1;
			inline constexpr int interrupt = 2;
			inline constexpr int quit      = 3;
			inline constexpr int abort     = 6;
			inline constexpr int kill      = 9;
			inline constexpr int segv      = 11;
			inline constexpr int pipe      = 13;
			inline constexpr int alarm     = 14;
			inline constexpr int terminate = 15;

#if ca_platform(linux)
			inline constexpr int user1     = 10;
			inline constexpr int user2     = 12;
			inline constexpr int child     = 17;
#else
			inline constexpr int user1     = 30;
			inline constexpr int user2     = 31;
			inline constexpr int child     = 20;
#endif

		} // namespace signals

	} // namespace sys

} // namespace ca

#endif // ca_system_process_signals_hpp
