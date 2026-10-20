// expect: exit 0
// mode: debug
#include "1-primitives/debug/trap.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "3-system/trace/backtrace.hpp"
#include "3-system/trace/of.hpp"
#include "4-lifecycle/with.hpp"

static ca::usz into_middle = 0U;
static ca::usz into_outer = 0U;

[[gnu::noinline]] static auto deepest(void) noexcept -> void {
	into_middle = reinterpret_cast<ca::usz>(__builtin_return_address(0)) - 1U;
	ca::trap("deep");
	asm volatile("" ::: "memory");
}

[[gnu::noinline]] static auto middle(void) noexcept -> void {
	into_outer = reinterpret_cast<ca::usz>(__builtin_return_address(0)) - 1U;
	deepest();
	asm volatile("" ::: "memory");
}

[[gnu::noinline]] static auto outer(void) noexcept -> void {
	middle();
	asm volatile("" ::: "memory");
}

auto main(void) -> int {
	auto trapped = ca::with([](void) noexcept { outer(); }, [](const ca::error*) noexcept {});
	if (trapped)
		return 1;
	const ca::borrowed_slice<const ca::usz> addresses = ca::trace::of(trapped.error()).backtrace();
	for (ca::usz first = 0U; first < 2U && first + 3U < addresses.size(); ++first) {
		if (addresses[first + 1U] == into_middle)
			return addresses[first + 2U] == into_outer ? 0 : 3;
	}
	return 2;
}
