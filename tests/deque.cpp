// expect: exit 0
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/array/deque.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/array/small_array.hpp"

static_assert(ca::deque<int>::TRIVIALLY_RELOCATABLE && sizeof(ca::deque<int>) == 4U * sizeof(void*));

template <typename D>
static auto in_order(const D& deque, const int from) noexcept -> bool {

	int expected = from;
	for (const auto& value : deque) {
		if (value != expected++)
			return false;
	}
	return expected - from == static_cast<int>(deque.size());
}

static auto queue(void) noexcept -> int {

	ca::deque<int> d;
	if (d.capacity() != 0U || d.first() || d.pop() || d.pop_front())
		return 1;
	for (int i = 0; i < 3; ++i)
		if (!d.append(i))
			return 2;
	for (int round = 0; round < 1002; ++round) {
		if (d.pop_front().value_or(-1) != round || !d.append(round + 3))
			return 3;
	}
	if (d.capacity() != 4U || !in_order(d, 1002) || d[2] != 1004 || d.get(3U))
		return 4;
	const auto runs = d.as_slices();
	if (runs[0U].size() + runs[1U].size() != 3U || runs[1U].empty())
		return 5;
	for (int i = 1001; i >= 992; --i)
		if (!d.prepend(i))
			return 6;
	if (!in_order(d, 992) || d.find(1002).value_or(0U) != 10U || d.count(997) != 1U || d.contains(1005))
		return 7;
	if (!d.extend({1005, 1006, 1007}) || !in_order(d, 992) || d.last().value_or(0) != 1007)
		return 8;
	if (!d.reserve(64U) || d.capacity() != 64U || !d.shrink() || d.capacity() != 16U || !in_order(d, 992) || !d.as_slices()[1U].empty())
		return 9;
	if (!d.reserve(100U) || d.capacity() != 128U || !in_order(d, 992))
		return 10;
	auto copy = d.clone();
	if (!copy || !in_order(*copy, 992) || copy->capacity() != 16U)
		return 11;
	return d.clear() && d.empty() && d.capacity() == 128U && d.destroy() && copy->destroy() && d.capacity() == 0U ? 0 : 12;
}

static auto wrapped_growth(void) noexcept -> int {

	ca::deque<ca::dynamic_array<int>> d;
	for (int i = 0; i < 40; ++i) {
		ca::dynamic_array<int> inner;
		if (!inner.append(i) || !(i % 2 == 0 ? d.append(ca::move(inner)) : d.prepend(ca::move(inner))))
			return 20;
		if (i % 7 == 6) {
			auto front = d.pop_front();
			if (!front || !front->destroy())
				return 21;
		}
	}
	ca::usz odd = 0U;
	for (const auto& inner : d)
		odd += inner[0] % 2 == 1 ? 1U : 0U;
	if (d.size() != 35U || odd != 15U)
		return 22;
	auto copy = d.clone();
	if (!copy || (*copy)[34][0] != d[34][0] || (*copy)[0].data() == d[0].data())
		return 23;
	ca::deque<ca::small_array<int, 1>> pinned;
	for (int i = 0; i < 100; ++i) {
		ca::small_array<int, 1> one;
		if (!one.append(i) || !(i % 5 == 0 ? pinned.append(ca::move(one)) : pinned.prepend(ca::move(one))))
			return 26;
	}
	int previous = 100;
	ca::usz backs = 0U;
	for (const auto& one : pinned) {
		if (one[0] % 5 != 0) {
			if (one[0] >= previous || backs != 0U)
				return 27;
			previous = one[0];
		}
		else
			++backs;
	}
	if (backs != 20U || pinned[99][0] != 95 || pinned[0][0] != 99 || !pinned.destroy())
		return 28;
	ca::deque<int> full;
	if (!full.extend({1, 2, 3, 4}) || full.size() != full.capacity() || !full.append(full[0]) || full.last().value_or(0) != 1)
		return 24;
	return d.destroy() && copy->destroy() && full.destroy() ? 0 : 25;
}

static auto arena(void) noexcept -> int {

	auto made = ca::arena::make();
	if (!made)
		return 30;
	ca::arena arena = ca::move(*made);
	{
		auto d = ca::make_in<ca::deque<int>>(arena);
		if (!d)
			return 31;
		for (int i = 0; i < 100; ++i)
			if (!d->prepend(99 - i))
				return 32;
		if (!in_order(*d, 0) || !d->destroy())
			return 33;
	}
	return arena.destroy() ? 0 : 34;
}

auto main(void) -> int {

	if (const int failed = queue(); failed != 0)
		return failed;
	if (const int failed = wrapped_growth(); failed != 0)
		return failed;
	return arena();
}
