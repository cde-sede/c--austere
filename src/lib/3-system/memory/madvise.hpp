#ifndef ca_system_memory_madvise_hpp
#define ca_system_memory_madvise_hpp

#include "1-primitives/integers/byte.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/memory/advice.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MADVISE --------------------------------------------------------

		/* @DOC Tells the kernel how region, page-aligned, will be used. */
		[[nodiscard]] auto madvise(ca::span<ca::byte> region, ca::sys::advice advice) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_madvise_hpp
