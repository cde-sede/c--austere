// expect: exit 42
// link: freestanding
#include "3-system/process/exit_immediately.hpp"

extern "C" auto atexit(void (*function)(void)) noexcept -> int;

static int destroyed = 0;

struct last final {
	~last(void) noexcept {
		destroyed = destroyed + 1;
	}
};

static last object;

static auto handler(void) noexcept -> void {
	ca::sys::exit_immediately(destroyed == 0 ? 42 : 1);
}

auto main(void) -> int {
	static_cast<void>(object);
	return atexit(&handler) == 0 ? 0 : 2;
}
