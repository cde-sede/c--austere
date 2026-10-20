// expect: exit 0
#include "1-primitives/debug/verdict.hpp"
#include "2-vocabulary/ask.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/fail.hpp"
#include "3-system/trace/of.hpp"
#include "4-lifecycle/context_manager.hpp"
#include "4-lifecycle/handler.hpp"
#include "4-lifecycle/trapped.hpp"
#include "4-lifecycle/with.hpp"

struct pool final {
	int free;
	int cached;
};

static auto take_one(pool& from) noexcept -> ca::result<void> {
	while (from.free == 0) {
		if (!ca::ask("pool empty", ca::errc::out_of_memory))
			return ca::trace::fail(ca::errc::out_of_memory, "pool empty");
	}
	--from.free;
	return {};
}

struct cache_ctx : ca::context_manager<cache_ctx> {
	pool* owned = nullptr;
	int asked = 0;

	auto handles(const ca::trapped& trap) noexcept -> ca::verdict {
		++this->asked;
		const auto error = trap.error();
		if (trap.resumable() && error.holds() && *error == ca::errc::out_of_memory && this->owned->cached > 0) {
			--this->owned->cached;
			++this->owned->free;
			return ca::verdict::resume;
		}
		return ca::verdict::pass;
	}

	auto enter(pool& from, const int count) noexcept -> void {
		this->owned = &from;
		for (int i = 0; i < count; ++i)
			take_one(from).raise();
	}

	auto leave(const ca::error*) noexcept -> void {
	}
};

auto main(void) -> int {
	pool outside{0, 3};
	if (take_one(outside) || outside.cached != 3)
		return 1;

	pool refilled{1, 2};
	cache_ctx cache{};
	auto three = cache.run(refilled, 3);
	if (!three || refilled.free != 0 || refilled.cached != 0 || cache.asked != 2)
		return 2;

	pool empty{0, 0};
	int caught = 0;
	auto outer = ca::with(
		[&](void) noexcept {
			cache_ctx inner{};
			static_cast<void>(inner.run(empty, 1));
		},
		[&caught](const ca::error* const error) noexcept { caught = error != nullptr && *error == ca::errc::out_of_memory; });
	if (outer || caught != 1)
		return 3;
	const ca::trace::record record = ca::trace::of(outer.error());
	if (record.frames().size() != 2U || record.frames()[0].message() != ca::sv::str{"pool empty"})
		return 4;

	pool aborted{0, 0};
	auto taken = ca::with(
		ca::handler{[](const ca::trapped& trap) noexcept { return trap.resumable() ? ca::verdict::take : ca::verdict::pass; }},
		[&aborted](void) noexcept { static_cast<void>(take_one(aborted)); },
		[](const ca::error*) noexcept {});
	if (taken || taken.error() != ca::errc::out_of_memory)
		return 5;
	if (ca::trace::of(taken.error()).frames()[0].message() != ca::sv::str{"pool empty"})
		return 6;
	return 0;
}
