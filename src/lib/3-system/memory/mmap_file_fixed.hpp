#ifndef ca_system_memory_mmap_file_fixed_hpp
#define ca_system_memory_mmap_file_fixed_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MMAP FILE FIXED ------------------------------------------------

		/* @DOC Maps the bytes of open file fd from offset (a multiple of page_size) over region,
		   page-aligned, replacing the pages there (MAP_FIXED), private copy-on-write, with
		   protection (sys::protection bits). For laying out a mapping reserved beforehand, as a
		   dynamic loader does; region must lie in memory the caller owns. */
		[[nodiscard]] auto mmap_file_fixed(ca::span<ca::byte> region, int fd, ca::u64 offset, int protection) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_mmap_file_fixed_hpp
