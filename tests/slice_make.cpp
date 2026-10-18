// expect: exit 0
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/array/slice.hpp"

static_assert(ca::slice<int>::TRIVIALLY_RELOCATABLE && sizeof(ca::slice<int>) == 3U * sizeof(void*));

auto main(void) -> int {
	auto sevens = ca::slice<int>::make(4U, 7);
	if (!sevens || sevens->size() != 4U || (*sevens)[3] != 7 || sevens->count(7) != 4U)
		return 1;
	auto zeros = ca::slice<int>::make(3U);
	auto none = ca::slice<int>::make(0U);
	if (!zeros || (*zeros)[2] != 0 || !none || none->data() != nullptr)
		return 2;
	const int values[4] = {4, 3, 2, 1};
	auto copy = ca::slice<int>::from(values);
	if (!copy || !(*copy == ca::borrowed_slice{values}))
		return 3;
	copy->reverse();
	if ((*copy)[0] != 1 || copy->find(4).value_or(0U) != 3U || copy->get(4U))
		return 4;
	auto twin = copy->clone();
	if (!twin || !(*twin == *copy) || twin->data() == copy->data())
		return 5;
	copy->prefix(2U).fill(9);
	if ((*copy)[1] != 9 || (*twin)[1] != 2)
		return 6;

	ca::dynamic_array<int> grown;
	for (int i = 0; i < 10; ++i)
		if (!grown.append(i))
			return 7;
	const int* const block = grown.data();
	ca::slice<int> fixed = ca::move(grown).into_slice();
	if (fixed.size() != 10U || fixed.data() != block || !grown.empty() || fixed.last().value_or(0) != 9)
		return 8;

	auto made = ca::arena::make();
	if (!made)
		return 9;
	ca::arena arena = ca::move(*made);
	{
		auto in_arena = ca::make_in<ca::slice<int>>(arena, 16U, 1);
		if (!in_arena || in_arena->size() != 16U || in_arena->count(1) != 16U || !in_arena->destroy())
			return 10;
	}
	if (!arena.destroy())
		return 11;
	return sevens->destroy() && zeros->destroy() && copy->destroy() && twin->destroy() && fixed.destroy() && fixed.empty() ? 0 : 12;
}
