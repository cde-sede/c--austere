// expect: exit 0
#include "1-primitives/debug/trap.hpp"
#include "1-primitives/debug/verdict.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/backtrace.hpp"
#include "3-system/trace/fail.hpp"
#include "3-system/trace/of.hpp"
#include "4-lifecycle/handler.hpp"
#include "4-lifecycle/trapped.hpp"
#include "4-lifecycle/with.hpp"

struct text final {
	char data[2048];
	ca::usz size;

	auto put(const char c) noexcept -> void {
		if (this->size < sizeof(this->data))
			this->data[this->size++] = c;
	}

	auto put(const ca::sv::str s) noexcept -> void {
		for (const char c : s)
			this->put(c);
	}

	auto contains(const ca::sv::str s) const noexcept -> bool {
		return ca::sv::str{this->data, this->size}.find(s).holds();
	}
};

static auto inside(const ca::usz address, auto (*function)(void) noexcept -> void) noexcept -> bool {

	const ca::usz begin = reinterpret_cast<ca::usz>(function);
	return address > begin && address < begin + 512U;
}

static auto near_top(const ca::borrowed_slice<const ca::usz> addresses, auto (*function)(void) noexcept -> void) noexcept -> bool {
	return (addresses.size() > 0U && inside(addresses[0], function)) || (addresses.size() > 1U && inside(addresses[1], function));
}

static ca::trace::backtrace captured{};

[[gnu::noinline]] static auto capturing(void) noexcept -> void {
	captured = ca::trace::backtrace::capture();
	asm volatile("" ::: "memory");
}

[[gnu::noinline]] static auto trapping(void) noexcept -> void {
	ca::trap("trapping");
	asm volatile("" ::: "memory");
}

[[gnu::noinline]] static auto failing(void) noexcept -> ca::result<int> {
	return ca::trace::fail(ca::errc::exists, "failing");
}

[[gnu::noinline]] static auto raising(void) noexcept -> void {
	failing().raise();
	asm volatile("" ::: "memory");
}

auto main(void) -> int {
	capturing();
	if (captured.empty() || !inside(captured.addresses()[0], &capturing))
		return 1;

	auto trapped = ca::with([](void) noexcept { trapping(); }, [](const ca::error*) noexcept {});
	if (trapped)
		return 2;
	const ca::trace::record trap_record = ca::trace::of(trapped.error());
	if (!near_top(trap_record.backtrace(), &trapping))
		return 2;

	auto raised = ca::with([](void) noexcept { raising(); }, [](const ca::error*) noexcept {});
	if (raised)
		return 3;
	const ca::trace::record raise_record = ca::trace::of(raised.error());
	if (!near_top(raise_record.backtrace(), &raising))
		return 3;
	if (raise_record.frames().size() != 2U)
		return 4;

	bool seen = false;
	auto handled = ca::with(
		ca::handler{[&seen](const ca::trapped& trap) noexcept {
			const ca::trace::backtrace here = trap.backtrace();
			seen = near_top(here.addresses(), &trapping);
			return ca::verdict::take;
		}},
		[](void) noexcept { trapping(); },
		[](const ca::error*) noexcept {});
	if (handled || !seen)
		return 5;

	if (!ca::trace::backtrace::walk(reinterpret_cast<const void*>(0x10)).empty())
		return 6;
	if (!ca::trace::backtrace::walk(reinterpret_cast<const void*>(0x1003)).empty())
		return 7;
	const ca::usz fake[2] = {0x7fffffffff00U, 0x1234U};
	const ca::trace::backtrace broken = ca::trace::backtrace::walk(fake);
	if (broken.size() != 1U || broken.addresses()[0] != 0x1233U)
		return 8;

	text printed{};
	trap_record.display(printed);
	if (!printed.contains(ca::sv::str{"\n  trapped: trapping"}) || !printed.contains(ca::sv::str{"\n  backtrace:\n    #0 exe+0x"}))
		return 9;
	auto plain = failing();
	if (plain || !ca::trace::of(plain.error()).backtrace().empty())
		return 10;
	return 0;
}
