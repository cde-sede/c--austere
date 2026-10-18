#ifndef ca_resources_print_flush_hpp
#define ca_resources_print_flush_hpp

#include "2-vocabulary/result.hpp"
#include "6-resources/io/out.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FLUSH --------------------------------------------------------------

	/* @DOC Writes out what print and printf buffered in this thread's stdout; the first error
	   since the last flush, if any. The explicit cleanup of stdout: exit's failsafe ignores
	   errors. */
	[[nodiscard]] inline auto flush(void) noexcept -> ca::result<void> {
		return ca::io::out().flush();
	}

} // namespace ca

#endif // ca_resources_print_flush_hpp
