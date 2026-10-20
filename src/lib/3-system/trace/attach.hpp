#ifndef ca_system_trace_attach_hpp
#define ca_system_trace_attach_hpp

#include "2-vocabulary/error.hpp"
#include "3-system/trace/backtrace.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE --------------------------------------------------------------

	namespace trace {


		// -- ATTACH ---------------------------------------------------------

		/* @DOC Gives error's trace backtrace, unless it has one already (the one nearest the
		   origin stays) or error does not carry the thread's trace. A context taking a trap
		   attaches the trap's backtrace this way. */
		auto attach(ca::error error, const ca::trace::backtrace& backtrace) noexcept -> void;

	} // namespace trace

} // namespace ca

#endif // ca_system_trace_attach_hpp
