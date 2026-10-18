#ifndef ca_collections_string_str_hpp
#define ca_collections_string_str_hpp

#include "2-vocabulary/sv/str.hpp"
#include "5-collections/string/basic_str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- STR ----------------------------------------------------------------

	/* @DOC Growable string of chars, UTF-8 by convention, from the heap: Python's str (see basic_str). make_in<str>(arena, text) puts one in an arena. */
	using str = ca::basic_str<char>;

} // namespace ca

#endif // ca_collections_string_str_hpp
