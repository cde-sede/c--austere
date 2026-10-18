#ifndef ca_system_trace_fail_hpp
#define ca_system_trace_fail_hpp

#include "1-primitives/debug/source_location.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE --------------------------------------------------------------

	namespace trace {


		// -- FAIL -----------------------------------------------------------

		/* @DOC Starts the thread's trace with error, first frame here (the caller), and returns it
		   to return: `return ca::trace::fail(ca::errc::no_entry, "no config");`. message must
		   outlive the trace: a literal. Replaces the previous trace; up to 32 frames are kept. */
		[[nodiscard, gnu::cold]] auto fail(ca::error error, ca::sv::str message = {}, ca::source_location where = ca::source_location::current()) noexcept -> ca::failure<ca::error>;

	} // namespace trace

} // namespace ca

#endif // ca_system_trace_fail_hpp
