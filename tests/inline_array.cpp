// expect: exit 0
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/traits/is_trivially_copyable.hpp"
#include "3-system/error/errc.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/array/inline_array.hpp"

static_assert(sizeof(ca::inline_array<int, 16>) == 72U && sizeof(ca::inline_array<ca::u64, 4>) == 40U);
static_assert(ca::is_trivially_copyable<ca::inline_array<int, 4>>);
static_assert(!ca::is_trivially_copyable<ca::inline_array<ca::dynamic_array<int>, 4>>);
static_assert(ca::inline_array<ca::dynamic_array<int>, 4>::TRIVIALLY_RELOCATABLE && ca::inline_array<int, 9>::capacity() == 9U);

template <typename T>
static auto fails_with(const ca::result<T>& result, const ca::error error) noexcept -> bool {
	return !result && result.error() == error;
}

static auto trivial(void) noexcept -> int {

	ca::inline_array<int, 4> a;
	if (!a.append(1) || !a.append(2) || !a.insert(0U, a[1]) || a.size() != 3U || a[0] != 2)
		return 1;
	if (!a.extend({9}) || !a.full() || !fails_with(a.append(5), ca::errc::no_space) || !fails_with(a.extend({1, 2}), ca::errc::no_space))
		return 2;
	ca::inline_array<int, 4> copy = a;
	copy[0] = 0;
	if (a[0] != 2 || copy == a)
		return 3;
	if (a.pop(1U).value_or(0) != 1 || a.swap_pop(0U).value_or(0) != 2 || a[0] != 9 || a.remove(2).value_or(0) != 2 || a.size() != 1U)
		return 4;
	if (!a.resize(4U, 6) || a.count(6) != 3U || !fails_with(a.resize(5U), ca::errc::no_space) || !a.resize(1U) || a.size() != 1U)
		return 5;
	if (!a.insert(0U, {1, 2}) || a[1] != 2 || a[2] != 9 || a.pop().value_or(0) != 9)
		return 6;
	return a.clear() && a.empty() && !a.pop() && fails_with(ca::inline_array<int, 2>::from({1, 2, 3}), ca::errc::no_space) ? 0 : 7;
}

static auto owning(void) noexcept -> int {

	ca::inline_array<ca::dynamic_array<int>, 3> lists;
	for (int i = 0; i < 3; ++i) {
		ca::dynamic_array<int> inner;
		if (!inner.append(i) || !lists.append(ca::move(inner)))
			return 10;
	}
	ca::dynamic_array<int> spare;
	if (!fails_with(lists.append(ca::move(spare)), ca::errc::no_space))
		return 11;
	auto copy = lists.clone();
	if (!copy || (*copy)[2][0] != 2 || (*copy)[2].data() == lists[2].data())
		return 12;
	const int* const block = lists[0].data();
	auto moved = ca::move(lists);
	if (!lists.empty() || moved.size() != 3U || moved[0].data() != block)
		return 13;
	auto first = moved.pop(0U);
	if (!first || (*first)[0] != 0 || moved[0][0] != 1 || !first->destroy())
		return 14;
	return moved.destroy() && copy->destroy() && moved.empty() ? 0 : 15;
}

auto main(void) -> int {

	if (const int failed = trivial(); failed != 0)
		return failed;
	return owning();
}
