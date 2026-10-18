#include "3-system/error/last_error.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		namespace impl {

			constinit thread_local int last_code = 0;

		} // namespace impl

	} // namespace sys

} // namespace ca
