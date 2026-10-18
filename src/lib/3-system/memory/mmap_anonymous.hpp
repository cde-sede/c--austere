#ifndef ca_system_memory_mmap_anonymous_hpp
#define ca_system_memory_mmap_anonymous_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MMAP ANONYMOUS -------------------------------------------------

		/* @DOC Maps size bytes of fresh zeroed memory, private to the process, page-aligned, with
		   protection (sys::protection bits). Release with munmap. */
		[[nodiscard]] auto mmap_anonymous(ca::usz size, int protection) noexcept -> ca::result<ca::span<ca::byte>>;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_mmap_anonymous_hpp
