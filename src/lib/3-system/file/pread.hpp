#ifndef ca_system_file_pread_hpp
#define ca_system_file_pread_hpp

#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- PREAD ----------------------------------------------------------

		/* @DOC Reads up to buffer.size() bytes at offset, leaving the file position alone; 0 past the
		   end. Retries on EINTR. */
		[[nodiscard]] auto pread(int fd, ca::span<ca::byte> buffer, ca::u64 offset) noexcept -> ca::result<ca::usz>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_pread_hpp
