#ifndef ca_collections_string_u8str_hpp
#define ca_collections_string_u8str_hpp

#include "2-vocabulary/sv/u8.hpp"
#include "5-collections/string/basic_str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- U8STR --------------------------------------------------------------

	/* @DOC Growable UTF-8 string of char8_t, from the heap. make_in<u8str>(arena, text) puts one in an arena. */
	using u8str = ca::basic_str<char8_t>;

} // namespace ca

#endif // ca_collections_string_u8str_hpp
