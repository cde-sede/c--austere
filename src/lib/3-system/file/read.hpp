#ifndef ca_system_file_read_hpp
#define ca_system_file_read_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- READ -----------------------------------------------------------

		/* @DOC Reads up to buffer.size() bytes into buffer; 0 at end of file. Retries on EINTR. */
		[[nodiscard]] auto read(int fd, ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_read_hpp
