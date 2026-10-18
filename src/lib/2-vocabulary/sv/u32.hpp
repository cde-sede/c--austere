#ifndef ca_vocabulary_sv_u32_hpp
#define ca_vocabulary_sv_u32_hpp

#include "2-vocabulary/sv/basic.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SV -----------------------------------------------------------------

	namespace sv {


		// -- U32 ------------------------------------------------------------

		/* @DOC Read-only run of UTF-32 code units: code points. */
		using u32 = ca::sv::basic<char32_t>;

	} // namespace sv

} // namespace ca

#endif // ca_vocabulary_sv_u32_hpp
