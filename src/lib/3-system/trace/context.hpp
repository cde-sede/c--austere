#ifndef ca_system_trace_context_hpp
#define ca_system_trace_context_hpp

#include "1-primitives/debug/source_location.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE --------------------------------------------------------------

	namespace trace {


		// -- CONTEXT --------------------------------------------------------

		/* @DOC Adds a frame here (the caller) to error's trace and returns error to return:
		   `if (!r) return ca::trace::context(r.error(), "opening config");`. An error without
		   the thread's current trace (made by hand, by a system call, by another thread) starts
		   one, as ca::trace::fail. message must outlive the trace: a literal. */
		[[nodiscard, gnu::cold]] auto context(ca::error error, ca::sv::str message = {}, ca::source_location where = ca::source_location::current()) noexcept -> ca::failure<ca::error>;

	} // namespace trace

} // namespace ca

#endif // ca_system_trace_context_hpp
