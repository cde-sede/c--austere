#ifndef ca_vocabulary_sv_u8_hpp
#define ca_vocabulary_sv_u8_hpp

#include "2-vocabulary/sv/basic.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SV -----------------------------------------------------------------

	namespace sv {


		// -- U8 -------------------------------------------------------------

		/* @DOC Read-only run of UTF-8 code units (char8_t). */
		using u8 = ca::sv::basic<char8_t>;

	} // namespace sv

} // namespace ca

#endif // ca_vocabulary_sv_u8_hpp
