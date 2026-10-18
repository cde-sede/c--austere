#ifndef ca_system_memory_munmap_hpp
#define ca_system_memory_munmap_hpp

#include "1-primitives/integers/byte.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MUNMAP ---------------------------------------------------------

		/* @DOC Unmaps mapping, as returned by mmap_anonymous or mmap_file (or pages of it). */
		[[nodiscard]] auto munmap(ca::span<ca::byte> mapping) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_munmap_hpp
