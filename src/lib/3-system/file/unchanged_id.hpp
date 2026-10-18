#ifndef ca_system_file_unchanged_id_hpp
#define ca_system_file_unchanged_id_hpp

#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- UNCHANGED ID ---------------------------------------------------

		/* @DOC User or group id that chown, fchown and lchown leave as it is. */
		inline constexpr ca::u32 unchanged_id = 0xFFFFFFFFU;

	} // namespace sys

} // namespace ca

#endif // ca_system_file_unchanged_id_hpp
