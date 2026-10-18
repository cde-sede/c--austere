#ifndef ca_system_file_readlink_hpp
#define ca_system_file_readlink_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- READLINK -------------------------------------------------------

		/* @DOC Writes symlink path's target into buffer, unterminated; returns its length. A length of
		   buffer.size() may mean truncation: retry larger. */
		[[nodiscard]] auto readlink(const char* path, ca::span<char> buffer) noexcept -> ca::result<ca::usz>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_readlink_hpp
