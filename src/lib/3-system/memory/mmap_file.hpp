#ifndef ca_system_memory_mmap_file_hpp
#define ca_system_memory_mmap_file_hpp

#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/memory/sharing.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MMAP FILE ------------------------------------------------------

		/* @DOC Maps size bytes of open file fd from offset (a multiple of page_size), page-aligned, with
		   protection (sys::protection bits). The mapping outlives fd. Release with munmap. */
		[[nodiscard]] auto mmap_file(int fd, ca::u64 offset, ca::usz size, int protection, ca::sys::sharing sharing) noexcept -> ca::result<ca::span<ca::byte>>;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_mmap_file_hpp
