#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "3-system/trace/impl/trace_state.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE STATE --------------------------------------------------------

	namespace trace {


		namespace impl {

			constinit thread_local ca::trace::impl::trace_state current{};

		} // namespace impl

	} // namespace trace

} // namespace ca


static constinit ca::atomic<ca::u32> last_id{0U}; // file-state: ignore


auto ca::trace::impl::start(const ca::error error, const ca::sv::str message, const ca::source_location where) noexcept -> ca::error {

	ca::u32 id = last_id.fetch_add<ca::memory_order::relaxed>(1U) + 1U;
	if (id == 0U) [[unlikely]]
		id = last_id.fetch_add<ca::memory_order::relaxed>(1U) + 1U;
	ca::trace::impl::trace_state& state = ca::trace::impl::current;
	     state._id = id;
	   state._code = error.code();
	   state._size = 0U;
	state._dropped = 0U;
	state._backtrace.clear();
	ca::trace::impl::push(message, where);
	return ca::error{error.code(), id};
}

auto ca::trace::impl::push(const ca::sv::str message, const ca::source_location where) noexcept -> void {

	ca::trace::impl::trace_state& state = ca::trace::impl::current;
	if (state._size == ca::trace::impl::CAPACITY) [[unlikely]] {
		++state._dropped;
		return;
	}
	state._frames[state._size++] = ca::trace::frame{message, where};
}
