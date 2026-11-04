#ifndef ca_collections_regex_regex_hpp
#define ca_collections_regex_regex_hpp

#include "4-lifecycle/memory/heap.hpp"
#include "5-collections/regex/basic_regex.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX --------------------------------------------------------------

	/* @DOC A compiled regular expression from the heap: Python's re.Pattern (see basic_regex).
	   make_in<regex>(arena, pattern) puts one in an arena. */
	using regex = ca::basic_regex<ca::heap>;

} // namespace ca

#endif // ca_collections_regex_regex_hpp
