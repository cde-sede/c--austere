#include "1-primitives/limits.hpp"
#include "3-system/sync/once.hpp"
#include "3-system/thread/futex_wait.hpp"
#include "3-system/thread/futex_wake.hpp"


[[nodiscard]] auto ca::once::_begin(void) noexcept -> bool {
	for (;;) {
		ca::u32 seen = 0U;
		if (_state.compare_exchange_strong<ca::memory_order::acquire>(seen, 1U))
			return true;
		if (seen == 3U)
			return false;
		if (seen == 1U && !_state.compare_exchange_strong<ca::memory_order::acquire>(seen, 2U))
			continue;
		ca::sys::futex_wait(_state, 2U);
	}
}

auto ca::once::_finish(const bool done) noexcept -> void {
	if (_state.exchange<ca::memory_order::release>(done ? 3U : 0U) == 2U)
		ca::sys::futex_wake(_state, ca::limits<ca::u32>::max());
}
