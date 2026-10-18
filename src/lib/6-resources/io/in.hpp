#ifndef ca_resources_io_in_hpp
#define ca_resources_io_in_hpp

#include "6-resources/io/out.hpp"
#include "6-resources/io/reader.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- IN -------------------------------------------------------------

		namespace impl {

			extern constinit ca::io::reader in_reader;

		} // namespace impl


		/* @DOC The process's standard input (fd 0), 4 KiB buffer, tied to the calling thread's
		   ca::io::out() so a pending prompt shows before input is awaited. One per process (a
		   buffer per thread would split stdin between threads): read it from one thread at a
		   time. */
		[[nodiscard]] inline auto in(void) noexcept -> ca::io::reader& {

			ca::io::impl::in_reader.tie(&ca::io::out());
			return ca::io::impl::in_reader;
		}

	} // namespace io

} // namespace ca

#endif // ca_resources_io_in_hpp
