#ifndef ca_system_file_mount_flags_hpp
#define ca_system_file_mount_flags_hpp

#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MOUNT FLAGS ----------------------------------------------------

		/* @DOC The flags argument of mount, Linux's MS_* values without <sys/mount.h>. Checked against
		   it when the library builds. */
		namespace mount_flags {

			inline constexpr ca::u64 read_only              = 0x1U;
			inline constexpr ca::u64 nosuid                 = 0x2U;
			inline constexpr ca::u64 nodev                  = 0x4U;
			inline constexpr ca::u64 noexec                 = 0x8U;
			inline constexpr ca::u64 synchronous            = 0x10U;
			inline constexpr ca::u64 remount                = 0x20U;
			inline constexpr ca::u64 mandatory_lock         = 0x40U;
			inline constexpr ca::u64 directory_sync         = 0x80U;
			inline constexpr ca::u64 nosymfollow            = 0x100U;
			inline constexpr ca::u64 noatime                = 0x400U;
			inline constexpr ca::u64 nodiratime             = 0x800U;
			inline constexpr ca::u64 bind                   = 0x1000U;
			inline constexpr ca::u64 move                   = 0x2000U;
			inline constexpr ca::u64 recursive              = 0x4000U;
			inline constexpr ca::u64 silent                 = 0x8000U;
			inline constexpr ca::u64 propagation_unbindable = 0x20000U;
			inline constexpr ca::u64 propagation_private    = 0x40000U;
			inline constexpr ca::u64 propagation_slave      = 0x80000U;
			inline constexpr ca::u64 propagation_shared     = 0x100000U;
			inline constexpr ca::u64 relatime               = 0x200000U;
			inline constexpr ca::u64 strictatime            = 0x1000000U;
			inline constexpr ca::u64 lazytime               = 0x2000000U;

		} // namespace mount_flags

	} // namespace sys

} // namespace ca

#endif // ca_system_file_mount_flags_hpp
