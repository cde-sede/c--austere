// expect: exit 139
// link: freestanding
#include "3-system/thread/thread.hpp"

[[gnu::noinline]] static auto descend(const int depth) noexcept -> int {

	volatile char frame[4096];
	frame[0] = static_cast<char>(depth);
	return depth == 1 << 30 ? 0 : descend(depth + 1) + frame[0];
}

static auto overflow(void*) noexcept -> void {
	static_cast<void>(descend(0));
}

auto main(void) -> int {
	auto started = ca::sys::thread::make(&overflow, nullptr, ca::sys::thread::MINIMUM_STACK_SIZE);
	if (!started)
		return 1;
	return started->join() ? 2 : 3;
}
