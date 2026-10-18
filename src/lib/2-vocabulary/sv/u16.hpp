#ifndef ca_vocabulary_sv_u16_hpp
#define ca_vocabulary_sv_u16_hpp

#include "2-vocabulary/sv/basic.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SV -----------------------------------------------------------------

	namespace sv {


		// -- U16 ------------------------------------------------------------

		/* @DOC Read-only run of UTF-16 code units. */
		using u16 = ca::sv::basic<char16_t>;

	} // namespace sv

} // namespace ca

#endif // ca_vocabulary_sv_u16_hpp
