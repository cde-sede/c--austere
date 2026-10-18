#ifndef ca_system_io_readv_hpp
#define ca_system_io_readv_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/io/io_buffer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- READV ----------------------------------------------------------

		/* @DOC Reads into buffers in order, filling each before the next; 0 at end of file. count is at most 1024 (IOV_MAX). Retries on EINTR. */
		[[nodiscard]] auto readv(int fd, ca::span<const ca::sys::io_buffer> buffers) noexcept -> ca::result<ca::usz>;

	} // namespace sys

} // namespace ca

#endif // ca_system_io_readv_hpp
