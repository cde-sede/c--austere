#ifndef ca_system_file_umask_hpp
#define ca_system_file_umask_hpp

#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- UMASK ----------------------------------------------------------

		/* @DOC Sets the mask of permission bits that file creation clears; returns the previous one. Never
		   fails; process-wide. */
		[[nodiscard]] auto umask(ca::u32 mask) noexcept -> ca::u32;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_umask_hpp
