#ifndef ca_system_file_umount_flags_hpp
#define ca_system_file_umount_flags_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- UMOUNT FLAGS ---------------------------------------------------

		/* @DOC The flags argument of umount, Linux's MNT_* and UMOUNT_* values without <sys/mount.h>.
		   Checked against it when the library builds. */
		namespace umount_flags {

			inline constexpr int force    = 0x1;
			inline constexpr int detach   = 0x2;
			inline constexpr int expire   = 0x4;
			inline constexpr int nofollow = 0x8;

		} // namespace umount_flags

	} // namespace sys

} // namespace ca

#endif // ca_system_file_umount_flags_hpp
