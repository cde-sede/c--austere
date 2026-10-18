#ifndef ca_system_file_readlinkat_hpp
#define ca_system_file_readlinkat_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- READLINKAT -----------------------------------------------------

		/* @DOC readlink relative to directory dir: the target, unterminated, and its length. A length of
		   buffer.size() may mean truncation: retry larger. */
		[[nodiscard]] auto readlinkat(int dir, const char* path, ca::span<char> buffer) noexcept -> ca::result<ca::usz>;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_readlinkat_hpp
