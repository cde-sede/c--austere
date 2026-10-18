#ifndef ca_system_memory_protection_hpp
#define ca_system_memory_protection_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- PROTECTION -----------------------------------------------------

		/* @DOC Page protection bits for mmap_anonymous and mprotect, without <sys/mman.h>. Checked
		   against it when the library builds. */
		namespace protection {

			inline constexpr int none    = 0x0;
			inline constexpr int read    = 0x1;
			inline constexpr int write   = 0x2;
			inline constexpr int execute = 0x4;

		} // namespace protection

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_protection_hpp
