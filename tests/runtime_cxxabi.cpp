// expect: exit 42
// link: freestanding
// What C++ compiles to calls of, from libca_runtime when linked without libc: a function-local
// static initialized once, a thread_local's destructor run at exit before a static object's,
// a class with a pure virtual function, operator new and delete, plain and over-aligned.
#include "1-primitives/integers/usz.hpp"
#include "3-system/process/exit_immediately.hpp"

static int made = 0;
static bool thread_object_gone = false;
static volatile int seed = 5;

static auto make(void) noexcept -> int {
	made = made + 1;
	return seed * 2;
}

static auto shared(void) noexcept -> int {
	static const int value = make();
	return value;
}

struct thread_object final {
	int _value = seed;

	~thread_object(void) noexcept {
		thread_object_gone = true;
	}
};

static thread_local thread_object per_thread;

struct last final {
	~last(void) noexcept {
		ca::sys::exit_immediately(thread_object_gone ? 42 : 3);
	}
};

static last at_exit;

struct shape {
	virtual ~shape(void) noexcept = default;
	virtual auto sides(void) const noexcept -> int = 0;
};

struct square final : shape {
	auto sides(void) const noexcept -> int override {
		return 4;
	}
};

struct alignas(64) line final {
	char _bytes[64];
};

auto main(void) -> int {
	static_cast<void>(at_exit);
	if (shared() != 10 || shared() != 10 || made != 1)
		return 1;
	if (per_thread._value != 5)
		return 2;
	const shape* const s = new square{};
	if (s->sides() != 4)
		return 4;
	delete s;
	int* const numbers = new int[1000];
	for (int i = 0; i < 1000; ++i)
		numbers[i] = i;
	if (numbers[999] != 999)
		return 5;
	delete[] numbers;
	line* const lines = new line[3];
	if (reinterpret_cast<ca::usz>(lines) % 64U != 0U)
		return 6;
	lines[2]._bytes[63] = 1;
	delete[] lines;
	line* const one = new line{};
	if (reinterpret_cast<ca::usz>(one) % 64U != 0U)
		return 7;
	delete one;
	return 0;
}
