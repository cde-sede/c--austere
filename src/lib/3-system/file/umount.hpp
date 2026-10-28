#ifndef ca_system_file_umount_hpp
#define ca_system_file_umount_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- UMOUNT ---------------------------------------------------------

		/* @DOC Detaches the file system mounted at target, with umount_flags (umount2). errc::busy while
		   in use unless detach; errc::not_supported outside Linux. */
		[[nodiscard]] auto umount(const char* target, int flags) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_umount_hpp
