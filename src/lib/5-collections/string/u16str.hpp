#ifndef ca_collections_string_u16str_hpp
#define ca_collections_string_u16str_hpp

#include "2-vocabulary/sv/u16.hpp"
#include "5-collections/string/basic_str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- U16STR -------------------------------------------------------------

	/* @DOC Growable UTF-16 string, from the heap. make_in<u16str>(arena, text) puts one in an arena. */
	using u16str = ca::basic_str<char16_t>;

} // namespace ca

#endif // ca_collections_string_u16str_hpp
