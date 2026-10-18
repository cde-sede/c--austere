#ifndef ca_system_file_write_hpp
#define ca_system_file_write_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- WRITE ----------------------------------------------------------

		/* @DOC Writes up to all of bytes; returns how many. Retries on EINTR. */
		[[nodiscard]] auto write(int fd, ca::span<const ca::byte> bytes) noexcept -> ca::result<ca::usz>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_write_hpp
