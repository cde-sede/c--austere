// expect: exit 0
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/fail.hpp"
#include "3-system/trace/of.hpp"
#include "4-lifecycle/context_manager.hpp"
#include "4-lifecycle/ctx.hpp"
#include "6-resources/fs/descriptor.hpp"

static auto checked(const bool fail) noexcept -> ca::result<int> {
	if (fail)
		return ca::trace::fail(ca::errc::exists, "checked");
	return 4;
}

struct open_ctx : ca::context_manager<open_ctx> {
	ca::fs::descriptor file = ca::fs::descriptor::adopt(-1);
	int value = 0;

	auto enter(const char* const path, const bool fail) noexcept -> void {
		auto number = checked(fail);
		number.raise();
		this->value = *number;
		auto opened = ca::fs::descriptor::open(path);
		opened.raise();
		this->file = *ca::move(opened);
	}

	auto leave(const ca::error*) noexcept -> void {
		if (this->file.is_open())
			this->file.cleanup().raise();
	}
};

auto main(void) -> int {
	open_ctx good{};
	auto opened = good.run("/proc/self/comm", false);
	if (!opened || good.value != 4 || good.file.is_open())
		return 1;

	auto missing = ca::ctx<open_ctx>::run("/nonexistent/ca/raise", false);
	if (missing || missing.error() != ca::errc::no_entry)
		return 2;
	const ca::trace::record record = ca::trace::of(missing.error());
	if (record.frames().size() != 1U || record.frames()[0].message() != ca::sv::str{"error raised"})
		return 3;
	if (ca::sv::str{record.frames()[0].where().function()} != ca::sv::str{"enter"} || ca::sv::str{record.frames()[0].where().file()} != ca::sv::str{"tests/context_raise.cpp"})
		return 6;

	auto failed = ca::ctx<open_ctx>::run("/proc/self/comm", true);
	if (failed || failed.error() != ca::errc::exists)
		return 4;
	const ca::trace::record chain = ca::trace::of(failed.error());
	if (chain.frames().size() != 2U || chain.frames()[0].message() != ca::sv::str{"checked"})
		return 5;

	ca::result<void> fine{};
	fine.raise();
	return 0;
}
