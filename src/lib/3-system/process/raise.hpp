#ifndef ca_system_process_raise_hpp
#define ca_system_process_raise_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- RAISE ----------------------------------------------------------

		/* @DOC Sends signal to the calling thread; a handler runs before raise returns. */
		[[nodiscard]] auto raise(int signal) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_raise_hpp
