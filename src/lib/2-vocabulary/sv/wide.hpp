#ifndef ca_vocabulary_sv_wide_hpp
#define ca_vocabulary_sv_wide_hpp

#include "2-vocabulary/sv/basic.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SV -----------------------------------------------------------------

	namespace sv {


		// -- WIDE -----------------------------------------------------------

		/* @DOC Read-only run of wchar_t: UTF-32 on Linux and macOS, UTF-16 on Windows. */
		using wide = ca::sv::basic<wchar_t>;

	} // namespace sv

} // namespace ca

#endif // ca_vocabulary_sv_wide_hpp
