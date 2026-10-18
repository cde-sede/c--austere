#ifndef ca_collections_string_u32str_hpp
#define ca_collections_string_u32str_hpp

#include "2-vocabulary/sv/u32.hpp"
#include "5-collections/string/basic_str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- U32STR -------------------------------------------------------------

	/* @DOC Growable UTF-32 string, from the heap. make_in<u32str>(arena, text) puts one in an arena. */
	using u32str = ca::basic_str<char32_t>;

} // namespace ca

#endif // ca_collections_string_u32str_hpp
