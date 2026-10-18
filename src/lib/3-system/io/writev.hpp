#ifndef ca_system_io_writev_hpp
#define ca_system_io_writev_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/io/io_buffer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- WRITEV ---------------------------------------------------------

		/* @DOC Writes buffers in order, as one write; returns how many bytes. count is at most 1024 (IOV_MAX). Retries on EINTR. */
		[[nodiscard]] auto writev(int fd, ca::span<const ca::sys::io_buffer> buffers) noexcept -> ca::result<ca::usz>;

	} // namespace sys

} // namespace ca

#endif // ca_system_io_writev_hpp
