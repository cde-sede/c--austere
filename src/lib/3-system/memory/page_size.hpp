#ifndef ca_system_memory_page_size_hpp
#define ca_system_memory_page_size_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- PAGE SIZE ------------------------------------------------------

		/* @DOC Size of a memory page. Never fails. */
		[[nodiscard]] auto page_size(void) noexcept -> ca::usz;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_page_size_hpp
