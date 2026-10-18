#ifndef ca_system_file_seek_hpp
#define ca_system_file_seek_hpp

#include "1-primitives/integers/i64.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/file/seek_origin.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SEEK -----------------------------------------------------------

		/* @DOC Moves fd's offset; returns the new one, from the start. */
		[[nodiscard]] auto seek(int fd, ca::i64 offset, ca::sys::seek_origin origin) noexcept -> ca::result<ca::i64>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_seek_hpp
