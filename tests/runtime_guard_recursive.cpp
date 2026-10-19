// expect: trap
// link: freestanding
// A function-local static whose initializer reaches its own initialization again: libca_runtime's
// guard sees its own thread running the initializer and traps instead of waiting on itself.

static volatile int depth = 0;

static auto value(void) noexcept -> int;

static auto again(void) noexcept -> int {
	depth = depth + 1;
	return value() + 1;
}

static auto value(void) noexcept -> int {
	static const int computed = again();
	return computed;
}

auto main(void) -> int {
	return value() == 0 ? 1 : 2;
}
