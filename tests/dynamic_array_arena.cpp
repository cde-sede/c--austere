// expect: exit 0
#include "1-primitives/traits/is_default_constructible.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/array/dynamic_array.hpp"

using arena_array = ca::dynamic_array<int, ca::arena_allocator>;

static_assert(!ca::is_default_constructible<arena_array>);
static_assert(sizeof(arena_array) == 4U * sizeof(void*));
static_assert(sizeof(ca::dynamic_array<int>) == 3U * sizeof(void*));

auto main(void) -> int {

	auto made = ca::arena::make();
	if (!made)
		return 1;
	ca::arena arena = ca::move(*made);
	arena_array a{arena.allocator()};
	for (int i = 0; i < 10000; ++i)
		if (!a.append(i))
			return 2;
	if (a.size() != 10000 || a[9999] != 9999)
		return 3;
	auto copy = a.clone();
	if (!copy || (*copy)[5000] != 5000)
		return 4;
	arena_array moved = ca::move(a);
	if (!a.empty() || moved[1] != 1)
		return 5;
	a = ca::move(*copy);
	if (a[42] != 42)
		return 6;
	auto heap_copy = ca::dynamic_array<int>{}.clone();
	if (!heap_copy)
		return 7;
	if (!a.cleanup() || !moved.cleanup())
		return 8;
	return arena.cleanup() ? 0 : 9;
}
