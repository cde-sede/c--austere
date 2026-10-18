#ifndef ca_resources_io_buffering_hpp
#define ca_resources_io_buffering_hpp

#include "1-primitives/integers/u8.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- BUFFERING ------------------------------------------------------

		/* @DOC When a writer writes its buffer out: full when it fills (or on flush), line also after
		   every message holding a newline, none after every message. */
		enum class buffering : ca::u8 {
			full,
			line,
			none,
		};

	} // namespace io

} // namespace ca

#endif // ca_resources_io_buffering_hpp
