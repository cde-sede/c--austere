// expect: exit 0
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"

auto open(const int code) noexcept -> ca::result<int> {
	if (code != 0)
		return ca::failure{ca::error{code}};
	return 7;
}

auto main(void) -> int {
	int calls = 0;
	const auto fallback = [&calls](const ca::error error) noexcept -> ca::result<int> {
		++calls;
		return error == ca::errc::no_entry ? -1 : -2;
	};

	auto recovered = open(ca::errc::no_entry.code()).except(ca::errc::no_entry, fallback);
	if (!recovered || recovered.value() != -1 || calls != 1)
		return 1;

	auto other = open(ca::errc::access_denied.code()).except(ca::errc::no_entry, fallback);
	if (other || other.error() != ca::errc::access_denied || calls != 1)
		return 2;

	auto value = open(0).except(ca::errc::no_entry, fallback);
	if (!value || value.value() != 7 || calls != 1)
		return 3;

	const ca::result<int> failed = open(ca::errc::exists.code());
	auto chained = failed.except(ca::errc::no_entry, fallback).except(ca::errc::exists, fallback);
	if (!chained || chained.value() != -2 || calls != 2)
		return 4;

	ca::result<void> v{ca::failure{ca::errc::interrupted}};
	auto retried = ca::move(v).except(ca::errc::interrupted, [](ca::error) noexcept {
		return ca::result<void>{};
	});
	if (!retried)
		return 5;

	constexpr int folded = ca::result<int>{ca::failure{ca::errc::no_entry}}
		.except(ca::errc::no_entry, [](ca::error) noexcept { return ca::result<int>{3}; })
		.value_or(0);
	static_assert(folded == 3);
	return 0;
}
