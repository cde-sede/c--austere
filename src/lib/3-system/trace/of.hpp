#ifndef ca_system_trace_of_hpp
#define ca_system_trace_of_hpp

#include "2-vocabulary/error.hpp"
#include "3-system/trace/record.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE --------------------------------------------------------------

	namespace trace {


		// -- OF -------------------------------------------------------------

		/* @DOC error with its trace, when it is the thread's current one; with no frames
		   otherwise. */
		[[nodiscard]] auto of(ca::error error) noexcept -> ca::trace::record;

	} // namespace trace

} // namespace ca

#endif // ca_system_trace_of_hpp
