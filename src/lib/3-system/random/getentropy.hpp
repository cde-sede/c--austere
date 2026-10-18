#ifndef ca_system_random_getentropy_hpp
#define ca_system_random_getentropy_hpp

#include "1-primitives/integers/byte.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- GETENTROPY -----------------------------------------------------

		/* @DOC Fills buffer from the kernel's secure random source, any size (fetched 256 bytes
		   at a time). For keys and seeds. */
		[[nodiscard]] auto getentropy(ca::span<ca::byte> buffer) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_random_getentropy_hpp
