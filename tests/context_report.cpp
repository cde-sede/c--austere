// expect: exit 0
#include "1-primitives/debug/trap.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/fail.hpp"
#include "4-lifecycle/context_manager.hpp"
#include "4-lifecycle/ctx.hpp"
#include "4-lifecycle/with.hpp"
#include "5-collections/trace/context.hpp"
#include "5-collections/trace/report.hpp"

static auto reported(void) noexcept -> ca::result<int, ca::report<>> {
	return ca::context(ca::errc::exists, "reading {}", 3);
}

static auto plain(void) noexcept -> ca::result<int> {
	return ca::trace::fail(ca::errc::no_entry, "plain");
}

struct report_ctx : ca::context_manager<report_ctx, ca::report<>> {
	ca::usz frames = 0U;
	ca::error seen{};

	auto enter(const int which) noexcept -> void {
		if (which == 0)
			reported().raise();
		if (which == 1)
			plain().raise();
		ca::trap("other");
	}

	auto leave(const ca::report<>* const error) noexcept -> void {
		 this->seen = error->error();
		this->frames = error->size();
	}
};

auto main(void) -> int {
	report_ctx moved{};
	auto whole = moved.run(0);
	if (whole || moved.seen != ca::errc::exists || moved.frames != 1U)
		return 1;
	if (whole.error().frame(0U).message() != ca::sv::str{"reading 3"})
		return 2;
	static_cast<void>(whole.error().cleanup());

	report_ctx converted{};
	auto from_error = converted.run(1);
	if (from_error || converted.seen != ca::errc::no_entry || converted.frames != 2U)
		return 3;
	if (from_error.error().frame(1U).message() != ca::sv::str{"error raised"})
		return 4;
	static_cast<void>(from_error.error().cleanup());

	auto other = ca::ctx<report_ctx>::run(2);
	if (other || other.error().error() != ca::errc::not_recoverable || other.error().frame(0U).message() != ca::sv::str{"other"})
		return 5;
	static_cast<void>(other.error().cleanup());

	auto as_error = ca::with([](void) noexcept { reported().raise(); }, [](const ca::error*) noexcept {});
	if (as_error || as_error.error() != ca::errc::exists)
		return 6;

	auto with_report = ca::with<ca::report<>>([](void) noexcept { plain().raise(); }, [](const ca::report<>*) noexcept {});
	if (with_report || with_report.error().error() != ca::errc::no_entry)
		return 7;
	static_cast<void>(with_report.error().cleanup());
	return 0;
}
