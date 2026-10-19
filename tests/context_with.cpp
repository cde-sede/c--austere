// expect: exit 0
#include "1-primitives/debug/trap.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/of.hpp"
#include "4-lifecycle/with.hpp"
#include "5-collections/array/dynamic_array.hpp"

[[gnu::noinline]] static auto always_traps(void) noexcept -> void {
	ca::trap("always");
}

auto main(void) -> int {
	int entered = 0;
	int left = 0;
	const ca::error* seen = nullptr;
	auto leave = [&left, &seen](const ca::error* const error) noexcept {
		++left;
		seen = error;
	};

	auto ok = ca::with([&entered](const int n) noexcept { entered = n; }, leave, 3);
	if (!ok || entered != 3 || left != 1 || seen != nullptr)
		return 1;

	auto trapped = ca::with([](void) noexcept { ca::trap("boom"); }, leave);
	if (trapped || trapped.error() != ca::errc::not_recoverable || left != 2 || seen == nullptr)
		return 2;
	const ca::trace::record record = ca::trace::of(trapped.error());
	if (record.frames().size() != 1U || record.frames()[0].message() != ca::sv::str{"boom"})
		return 3;

	ca::dynamic_array<int> values{};
	auto out_of_range = ca::with([&values](void) noexcept { values[4] = 1; }, leave);
	if (out_of_range || ca::trace::of(out_of_range.error()).frames()[0].message() != ca::sv::str{"index out of bounds"})
		return 4;

	int inner_failed = 0;
	int after_inner = 0;
	auto outer = ca::with(
		[&](void) noexcept {
			auto inner = ca::with([](void) noexcept { ca::trap(); }, [](const ca::error*) noexcept {});
			inner_failed = !inner;
			after_inner = 1;
			always_traps();
		},
		leave);
	if (outer || inner_failed != 1 || after_inner != 1)
		return 5;

	int written = 0;
	auto stored = ca::with(
		[&written](void) noexcept {
			written = 7;
			always_traps();
		},
		[&written, &seen](const ca::error* const error) noexcept {
			seen = error;
			written += 1;
		});
	if (stored || written != 8)
		return 6;

	auto returned = ca::with([](void) noexcept -> ca::result<void> { return ca::failure{ca::errc::exists}; }, leave);
	if (returned || returned.error() != ca::errc::exists)
		return 7;
	return 0;
}
