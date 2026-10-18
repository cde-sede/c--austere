#ifndef ca_resources_io_out_hpp
#define ca_resources_io_out_hpp

#include "6-resources/io/writer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- OUT ------------------------------------------------------------

		namespace impl {

			extern constinit thread_local ca::io::writer* out_current;

			[[nodiscard, gnu::cold, gnu::noinline]] auto open_out(void) noexcept -> ca::io::writer&;

		} // namespace impl


		/* @DOC This thread's standard output (fd 1), 4 KiB buffer, line policy on a terminal, full
		   otherwise. Opened on first use. Flushed when the thread exits and, for the thread
		   calling exit() (returning from main included), at exit; ca::sys::exit_immediately and
		   a crash drop it. Per thread, so no lock: each message reaches fd 1 whole, in the order
		   its thread flushes. After fork(), flush in one process only. */
		[[nodiscard]] inline auto out(void) noexcept -> ca::io::writer& {

			ca::io::writer* const current = ca::io::impl::out_current;
			if (current == nullptr) [[unlikely]]
				return ca::io::impl::open_out();
			return *current;
		}

	} // namespace io

} // namespace ca

#endif // ca_resources_io_out_hpp
