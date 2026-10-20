#include "3-system/trace/attach.hpp"
#include "3-system/trace/impl/trace_state.hpp"


auto ca::trace::attach(const ca::error error, const ca::trace::backtrace& backtrace) noexcept -> void {

	ca::trace::impl::trace_state& state = ca::trace::impl::current;
	if (error.trace() == 0U || error.trace() != state._id || !state._backtrace.empty())
		return;
	state._backtrace = backtrace;
}
