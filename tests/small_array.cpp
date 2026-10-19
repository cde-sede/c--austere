// expect: exit 0
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "2-vocabulary/fail.hpp"
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/array/small_array.hpp"

static_assert(sizeof(ca::small_array<int, 4>) == 3U * sizeof(void*) + 4U * sizeof(int));
static_assert(!ca::is_trivially_relocatable<ca::small_array<int, 4>>);

static auto ints(void) noexcept -> int {

	ca::small_array<int, 4> a;
	if (!a.is_inline() || a.capacity() != 4U || a.pop())
		return 1;
	if (!a.extend({1, 2, 3}) || !a.append(a[0]) || !a.is_inline() || a.size() != 4U)
		return 2;
	if (!a.append(a[3]) || a.is_inline() || a.size() != 5U || a[4] != 1 || a.capacity() < 8U)
		return 3;
	if (!a.insert(0U, a[2]) || a[0] != 3 || !a.insert(1U, {7, 8}) || a.size() != 8U || a[2] != 8)
		return 4;
	if (a.pop(1U).value_or(0) != 7 || a.swap_pop(0U).value_or(0) != 3 || a.remove(8).value_or(0) != 8 || a.size() != 5U)
		return 5;
	if (!a.resize(3U) || !a.shrink() || !a.is_inline() || a.size() != 3U || a.capacity() != 4U)
		return 6;
	const int* const inline_data = a.data();
	ca::small_array<int, 4> moved = ca::move(a);
	if (!a.empty() || !a.is_inline() || !moved.is_inline() || moved.data() == inline_data || moved.size() != 3U)
		return 7;
	if (!moved.resize(20U, 9) || moved.count(9) != 17U)
		return 8;
	const int* const block = moved.data();
	ca::small_array<int, 4> taken = ca::move(moved);
	if (taken.data() != block || !moved.is_inline() || !moved.empty())
		return 9;
	auto copy = taken.clone();
	if (!copy || !(*copy == taken) || copy->data() == taken.data())
		return 10;
	auto small_copy = a.clone();
	if (!small_copy || !small_copy->is_inline())
		return 11;
	if (!taken.reserve(100U) || taken.capacity() < 100U || taken.count(9) != 17U)
		return 12;
	return taken.cleanup() && taken.is_inline() && copy->cleanup() && small_copy->cleanup() && a.cleanup() ? 0 : 13;
}

static int cleanups = 0;

struct cleanable {
	int code;
	auto cleanup(void) noexcept -> ca::result<void> {
		++cleanups;
		if (code != 0)
			return ca::fail(code);
		return ca::result<void>{};
	}
};

static auto owning(void) noexcept -> int {

	ca::small_array<ca::dynamic_array<int>, 2> nested;
	for (int i = 0; i < 5; ++i) {
		ca::dynamic_array<int> inner;
		if (!inner.append(i) || !nested.append(ca::move(inner)))
			return 20;
	}
	auto copy = nested.clone();
	if (!copy || (*copy)[4][0] != 4 || (*copy)[4].data() == nested[4].data())
		return 21;
	ca::small_array<cleanable, 2> parts;
	if (!parts.append(cleanable{0}) || !parts.append(cleanable{6}) || !parts.append(cleanable{7}))
		return 22;
	auto cleaned = parts.cleanup();
	if (cleaned || cleaned.error().code() != 6 || cleanups != 3 || !parts.is_inline())
		return 23;
	return nested.cleanup() && copy->cleanup() ? 0 : 24;
}

static auto arena(void) noexcept -> int {

	auto made = ca::arena::make();
	if (!made)
		return 30;
	ca::arena arena = ca::move(*made);
	{
		auto a = ca::make_in<ca::small_array<int, 8>>(arena);
		if (!a)
			return 31;
		for (int i = 0; i < 100; ++i)
			if (!a->append(i))
				return 32;
		if (a->is_inline() || (*a)[99] != 99 || !a->cleanup())
			return 33;
	}
	return arena.cleanup() ? 0 : 34;
}

auto main(void) -> int {

	if (const int failed = ints(); failed != 0)
		return failed;
	if (const int failed = owning(); failed != 0)
		return failed;
	return arena();
}
