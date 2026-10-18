// expect: exit 0
#include "1-primitives/traits/is_trivially_copyable.hpp"
#include "2-vocabulary/fail.hpp"
#include "5-collections/array/array.hpp"
#include "5-collections/array/dynamic_array.hpp"

static_assert(sizeof(ca::array<int, 5>) == 5U * sizeof(int) && ca::is_trivially_copyable<ca::array<int, 5>>);
static_assert(ca::array<int, 0>::empty() && ca::array<int, 0>{}.data() == nullptr);
static_assert(ca::array<ca::dynamic_array<int>, 2>::TRIVIALLY_RELOCATABLE);

constexpr auto build(void) noexcept -> bool {

	ca::array values{3, 1, 2};
	ca::array<int, 3> other = {2, 1, 3};
	values.reverse();
	const ca::array<int, 4> zeros;
	return values == other && values[0] == 2 && !values.get(3U) && values.find(3).value_or(9U) == 2U
		&& values.prefix(2U).size() == 2U && zeros.count(0) == 4U && values.clone().value_or(zeros.prefix(3U).size() == 3U ? other : values) == other;
}

static_assert(build());

static int cleanups = 0;

struct cleanable {
	int code;
	auto destroy(void) noexcept -> ca::result<void> {
		++cleanups;
		if (code != 0)
			return ca::fail(code);
		return ca::result<void>{};
	}
};

auto main(void) -> int {

	if (!build())
		return 1;
	ca::array<cleanable, 3> parts{cleanable{0}, cleanable{4}, cleanable{5}};
	auto cleaned = parts.destroy();
	if (cleaned || cleaned.error().code() != 4 || cleanups != 3)
		return 2;

	ca::array<ca::dynamic_array<int>, 2> nested;
	if (!nested[0].append(7) || !nested[1].append(8))
		return 3;
	auto copy = nested.clone();
	if (!copy || (*copy)[1][0] != 8 || (*copy)[0].data() == nested[0].data())
		return 4;
	int total = 0;
	for (const auto& inner : *copy)
		total += inner.first().value_or(0);
	return total == 15 && nested.destroy() && copy->destroy() ? 0 : 5;
}
