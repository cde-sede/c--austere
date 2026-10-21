#include "1-primitives/debug/trap.hpp"
#include "1-primitives/limits.hpp"
#include "3-system/sync/latch.hpp"
#include "3-system/thread/futex_wait.hpp"
#include "3-system/thread/futex_wait_until.hpp"
#include "3-system/thread/futex_wake.hpp"


[[noreturn, gnu::cold, gnu::noinline]] static auto latch_counted_below_zero(void) noexcept -> void {
	ca::trap("latch counted below zero");
}

auto ca::latch::count_down(const ca::u32 n) noexcept -> void {

	const ca::u32 previous = _count.fetch_sub<ca::memory_order::acq_rel>(n);
	if (previous < n) [[unlikely]]
		latch_counted_below_zero();
	if (previous == n)
		ca::sys::futex_wake(_count, ca::limits<ca::u32>::max());
}

auto ca::latch::wait(void) const noexcept -> void {
	for (ca::u32 left = _count.load<ca::memory_order::acquire>(); left != 0U; left = _count.load<ca::memory_order::acquire>())
		ca::sys::futex_wait(_count, left);
}

[[nodiscard]] auto ca::latch::wait_until(const ca::i64 deadline) const noexcept -> ca::result<void> {

	for (ca::u32 left = _count.load<ca::memory_order::acquire>(); left != 0U; left = _count.load<ca::memory_order::acquire>()) {
		auto waited = ca::sys::futex_wait_until(_count, left, deadline);
		if (!waited) [[unlikely]]
			return try_wait() ? ca::result<void>{} : waited;
	}
	return ca::result<void>{};
}
