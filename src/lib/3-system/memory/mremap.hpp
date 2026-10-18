#ifndef ca_system_memory_mremap_hpp
#define ca_system_memory_mremap_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/memory/remapping.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MREMAP ---------------------------------------------------------

		/* @DOC Resizes page-aligned mapping to size bytes, keeping its pages: a move relocates them
		   without copying. Returns the resized mapping; on failure it is untouched. Linux only,
		   errc::not_supported elsewhere. */
		[[nodiscard]] auto mremap(ca::span<ca::byte> mapping, ca::usz size, ca::sys::remapping remapping) noexcept -> ca::result<ca::span<ca::byte>>;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_mremap_hpp
