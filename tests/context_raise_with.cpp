// expect: exit 0
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/context.hpp"
#include "3-system/trace/fail.hpp"
#include "3-system/trace/of.hpp"
#include "4-lifecycle/with.hpp"
#include "5-collections/trace/context.hpp"
#include "5-collections/trace/report.hpp"

static auto checked(const bool fail) noexcept -> ca::result<int> {
	if (fail)
		return ca::trace::fail(ca::errc::exists, "checked");
	return 4;
}

static auto as_no_entry(ca::error) noexcept -> ca::failure<ca::error> {
	return ca::trace::fail(ca::errc::no_entry, "mapped");
}

auto main(void) -> int {
	int value = 0;
	auto chained = ca::with([&value](void) noexcept { value = *checked(false).raise() + *checked(false).raise(as_no_entry); }, [](const ca::error*) noexcept {});
	if (!chained || value != 8)
		return 1;

	auto contexted = ca::with(
		[](void) noexcept {
			checked(true).raise([](const ca::error e) noexcept { return ca::trace::context(e, "while loading"); });
		},
		[](const ca::error*) noexcept {});
	if (contexted || contexted.error() != ca::errc::exists)
		return 2;
	const ca::trace::record record = ca::trace::of(contexted.error());
	if (record.frames().size() != 3U || record.frames()[1].message() != ca::sv::str{"while loading"})
		return 3;

	auto mapped = ca::with([](void) noexcept { checked(true).raise(&as_no_entry); }, [](const ca::error*) noexcept {});
	if (mapped || mapped.error() != ca::errc::no_entry || ca::trace::of(mapped.error()).frames()[0].message() != ca::sv::str{"mapped"})
		return 4;

	auto reported = ca::with<ca::report<>>(
		[](void) noexcept {
			checked(true).raise([](const ca::error e) noexcept { return ca::context(e, "item {}", 7); });
		},
		[](const ca::report<>*) noexcept {});
	if (reported || reported.error().error() != ca::errc::exists || reported.error().frame(reported.error().size() - 1U).message() != ca::sv::str{"item 7"})
		return 5;
	static_cast<void>(reported.error().cleanup());
	return 0;
}
