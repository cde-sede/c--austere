#ifndef ca_resources_io_err_hpp
#define ca_resources_io_err_hpp

#include "6-resources/io/writer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- ERR ------------------------------------------------------------

		namespace impl {

			extern constinit thread_local ca::io::writer* err_current;

			[[nodiscard, gnu::cold, gnu::noinline]] auto open_err(void) noexcept -> ca::io::writer&;

		} // namespace impl


		/* @DOC This thread's standard error (fd 2), unbuffered: each message is one write as it
		   happens. Nothing is ever pending; flush() only reports the sticky error. */
		[[nodiscard]] inline auto err(void) noexcept -> ca::io::writer& {

			ca::io::writer* const current = ca::io::impl::err_current;
			if (current == nullptr) [[unlikely]]
				return ca::io::impl::open_err();
			return *current;
		}

	} // namespace io

} // namespace ca

#endif // ca_resources_io_err_hpp
