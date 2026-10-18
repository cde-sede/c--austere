#ifndef ca_system_file_pwrite_hpp
#define ca_system_file_pwrite_hpp

#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- PWRITE ---------------------------------------------------------

		/* @DOC Writes up to all of bytes at offset, leaving the file position alone; returns how
		   many. Retries on EINTR. Ignores offset when the file was opened with fs::append on Linux. */
		[[nodiscard]] auto pwrite(int fd, ca::span<const ca::byte> bytes, ca::u64 offset) noexcept -> ca::result<ca::usz>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_pwrite_hpp
