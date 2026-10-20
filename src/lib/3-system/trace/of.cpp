#include "3-system/trace/impl/trace_state.hpp"
#include "3-system/trace/of.hpp"


[[nodiscard]] auto ca::trace::of(const ca::error error) noexcept -> ca::trace::record {

	const ca::trace::impl::trace_state& state = ca::trace::impl::current;
	if (error.trace() == 0U || error.trace() != state._id)
		return ca::trace::record{error, {}, 0U};
	return ca::trace::record{error, ca::borrowed_slice<const ca::trace::frame>{state._frames, state._size}, state._dropped, &state._backtrace};
}
