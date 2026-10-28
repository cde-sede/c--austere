#ifndef ca_system_file_mount_hpp
#define ca_system_file_mount_hpp

#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MOUNT ----------------------------------------------------------

		/* @DOC Attaches the file system source of kind type at directory target, with mount_flags and
		   file-system-specific data. type and data may be null where flags make them unused (bind,
		   move, remount, propagation changes). errc::not_permitted without CAP_SYS_ADMIN;
		   errc::not_supported outside Linux. */
		[[nodiscard]] auto mount(const char* source, const char* target, const char* type, ca::u64 flags, const void* data) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_mount_hpp
