#ifndef ca_collections_string_wstr_hpp
#define ca_collections_string_wstr_hpp

#include "2-vocabulary/sv/wide.hpp"
#include "5-collections/string/basic_str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- WSTR ---------------------------------------------------------------

	/* @DOC Growable string of wchar_t, from the heap. make_in<wstr>(arena, text) puts one in an arena. */
	using wstr = ca::basic_str<wchar_t>;

} // namespace ca

#endif // ca_collections_string_wstr_hpp
