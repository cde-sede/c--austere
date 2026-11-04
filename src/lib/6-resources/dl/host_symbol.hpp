#ifndef ca_resources_dl_host_symbol_hpp
#define ca_resources_dl_host_symbol_hpp

#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DL -----------------------------------------------------------------

	namespace dl {


		// -- HOST SYMBOL ----------------------------------------------------

		/* @DOC A symbol the program offers to the libraries it loads: a name and the address
		   of what it names, a function or a variable. Looked up first, before the program's
		   exported symbols and any library. */
		struct host_symbol final {

				using self = ca::dl::host_symbol;

				ca::sv::str _name;
				void* _address;

		}; // struct host_symbol

	} // namespace dl

} // namespace ca

#endif // ca_resources_dl_host_symbol_hpp
