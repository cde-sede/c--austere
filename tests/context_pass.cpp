// expect: exit 0
#include "1-primitives/debug/trap.hpp"
#include "1-primitives/debug/verdict.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/fail.hpp"
#include "3-system/trace/of.hpp"
#include "4-lifecycle/context_manager.hpp"
#include "4-lifecycle/handler.hpp"
#include "4-lifecycle/outcome.hpp"
#include "4-lifecycle/trapped.hpp"
#include "4-lifecycle/with.hpp"

static auto failing(const ca::error code) noexcept -> ca::result<int> {
	return ca::trace::fail(code, "failing");
}

struct log final {
	int order[4];
	ca::outcome outcomes[4];
	int count;

	auto add(const int who, const ca::outcome outcome) noexcept -> void {
		this->order[this->count] = who;
		this->outcomes[this->count] = outcome;
		++this->count;
	}
};

struct no_entry_ctx : ca::context_manager<no_entry_ctx> {
	log* seen = nullptr;
	int reached = 0;

	auto handles(const ca::trapped& trap) const noexcept -> ca::verdict {
		const auto error = trap.error();
		return error.holds() && *error == ca::errc::no_entry ? ca::verdict::take : ca::verdict::pass;
	}

	auto enter(log& into, const ca::error code) noexcept -> void {
		this->seen = &into;
		failing(code).raise();
		this->reached = 1;
	}

	auto leave(const ca::error* const error, const ca::outcome outcome) noexcept -> void {
		if ((error == nullptr) != (outcome == ca::outcome::succeeded))
			ca::trap("error and outcome disagree");
		this->seen->add(1, outcome);
	}
};

auto main(void) -> int {
	log seen{};
	no_entry_ctx taken{};
	auto own = taken.run(seen, ca::errc::no_entry);
	if (own || own.error() != ca::errc::no_entry || taken.reached != 0)
		return 1;
	if (seen.count != 1 || seen.outcomes[0] != ca::outcome::taken)
		return 2;

	seen = log{};
	int after_inner = 0;
	auto outer = ca::with(
		[&](void) noexcept {
			no_entry_ctx inner{};
			static_cast<void>(inner.run(seen, ca::errc::exists));
			after_inner = 1;
		},
		[&seen](const ca::error*, const ca::outcome outcome) noexcept { seen.add(2, outcome); });
	if (outer || outer.error() != ca::errc::exists || after_inner != 0)
		return 3;
	if (seen.count != 2 || seen.order[0] != 1 || seen.outcomes[0] != ca::outcome::passed
	 || seen.order[1] != 2 || seen.outcomes[1] != ca::outcome::taken)
		return 4;
	const ca::trace::record record = ca::trace::of(outer.error());
	if (record.frames().size() != 2U || record.frames()[0].message() != ca::sv::str{"failing"}
	 || record.frames()[1].message() != ca::sv::str{"error raised"})
		return 5;

	int one_argument = 0;
	ca::sv::str reason{};
	auto passing = ca::with(
		[&](void) noexcept {
			static_cast<void>(ca::with(
				ca::handler{[&reason](const ca::trapped& trap) noexcept {
					reason = trap.reason();
					return ca::verdict::pass;
				}},
				[](void) noexcept { ca::trap("boom"); },
				[&one_argument](const ca::error* const error) noexcept { one_argument = error != nullptr ? 1 : 2; }));
		},
		[](const ca::error*) noexcept {});
	if (passing || passing.error() != ca::errc::not_recoverable || one_argument != 1 || reason != ca::sv::str{"boom"})
		return 6;
	if (ca::trace::of(passing.error()).frames().size() != 1U)
		return 7;

	int middle = 0;
	auto skipped = ca::with(
		[&](void) noexcept {
			static_cast<void>(ca::with(
				[&](void) noexcept {
					static_cast<void>(ca::with(
						ca::handler{[](const ca::trapped&) noexcept { return ca::verdict::pass; }},
						[](void) noexcept { failing(ca::errc::no_entry).raise(); },
						[](const ca::error*) noexcept {}));
				},
				[&middle](const ca::error*, const ca::outcome outcome) noexcept { middle = outcome == ca::outcome::taken ? 1 : 2; }));
			middle += 10;
		},
		[](const ca::error*) noexcept {});
	if (!skipped || middle != 11)
		return 8;

	auto succeeded = ca::with([](void) noexcept {}, [&seen](const ca::error*, const ca::outcome outcome) noexcept { seen.add(3, outcome); });
	if (!succeeded || seen.order[2] != 3 || seen.outcomes[2] != ca::outcome::succeeded)
		return 9;
	return 0;
}
