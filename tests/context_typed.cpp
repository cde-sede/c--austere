// expect: exit 0
#include "1-primitives/debug/verdict.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/handler.hpp"
#include "4-lifecycle/outcome.hpp"
#include "4-lifecycle/trapped.hpp"
#include "4-lifecycle/with.hpp"

struct parse_error final {
	int column;

	static auto of(const ca::error) noexcept -> parse_error {
		return parse_error{-1};
	}
};

static auto parse(const int column) noexcept -> ca::result<int, parse_error> {
	return ca::failure{parse_error{column}};
}

auto main(void) -> int {
	int seen_column = 0;
	auto taken = ca::with<parse_error>(
		ca::handler{[](const ca::trapped& trap) noexcept { return trap.as<parse_error>() != nullptr ? ca::verdict::take : ca::verdict::pass; }},
		[](void) noexcept { parse(7).raise(); },
		[&seen_column](const parse_error* const error) noexcept { seen_column = error->column; });
	if (taken || taken.error().column != 7 || seen_column != 7)
		return 1;

	int passed_column = 0;
	const parse_error* passed_address = nullptr;
	auto outer = ca::with(
		[&](void) noexcept {
			static_cast<void>(ca::with<parse_error>(
				ca::handler{[](const ca::trapped& trap) noexcept { return trap.as<int>() != nullptr ? ca::verdict::take : ca::verdict::pass; }},
				[](void) noexcept { parse(3).raise(); },
				[&](const parse_error* const error, const ca::outcome outcome) noexcept {
					if (outcome == ca::outcome::passed) {
						passed_column = error->column;
						passed_address = error;
					}
				}));
		},
		[](const ca::error*) noexcept {});
	if (outer || outer.error() != ca::errc::not_recoverable || passed_column != 3 || passed_address == nullptr)
		return 2;

	int converted = 0;
	auto other = ca::with(
		[&](void) noexcept {
			static_cast<void>(ca::with<parse_error>(
				ca::handler{[](const ca::trapped&) noexcept { return ca::verdict::pass; }},
				[](void) noexcept { ca::trap("not a parse error"); },
				[&converted](const parse_error* const error) noexcept { converted = error->column; }));
		},
		[](const ca::error*) noexcept {});
	if (other || converted != -1)
		return 3;
	return 0;
}
