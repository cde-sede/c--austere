#ifndef ca_system_file_at_hpp
#define ca_system_file_at_hpp

#include "1-primitives/config/platform.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- AT -------------------------------------------------------------

		/* @DOC Directory and flag arguments of the *at calls, without <fcntl.h>. Checked against it when
		   the library builds. */
		namespace at {

			/* @DOC The working directory, as the dir argument. */
		#if ca_platform(linux)
			inline constexpr int cwd              = -100;
			inline constexpr int symlink_nofollow = 0x100;
			inline constexpr int remove_directory = 0x200;
			inline constexpr int effective_ids    = 0x200;
			inline constexpr int symlink_follow   = 0x400;
		#else
			inline constexpr int cwd              = -2;
			inline constexpr int effective_ids    = 0x10;
			inline constexpr int symlink_nofollow = 0x20;
			inline constexpr int symlink_follow   = 0x40;
			inline constexpr int remove_directory = 0x80;
		#endif

		} // namespace at

	} // namespace sys

} // namespace ca

#endif // ca_system_file_at_hpp
