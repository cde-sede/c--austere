#ifndef ca_vocabulary_sv_str_hpp
#define ca_vocabulary_sv_str_hpp

#include "2-vocabulary/sv/basic.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SV -----------------------------------------------------------------

	namespace sv {


		// -- STR ------------------------------------------------------------

		/* @DOC Read-only run of chars, UTF-8 by convention: the everyday string view. */
		using str = ca::sv::basic<char>;

	} // namespace sv

} // namespace ca

#endif // ca_vocabulary_sv_str_hpp
