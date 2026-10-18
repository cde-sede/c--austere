// expect: exit 0
#include "1-primitives/integers/u8.hpp"
#include "4-lifecycle/memory/expand.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/reallocate.hpp"

auto main(void) -> int {

	const ca::heap heap{};
	auto block = heap.allocate(20U, 8U);
	if (!block)
		return 1;
	for (ca::usz i = 0U; i < 20U; ++i)
		static_cast<ca::u8*>(block->data())[i] = static_cast<ca::u8>(i);
	ca::allocation current = *block;
	for (ca::usz size = 24U; size < (ca::usz{64U} << 20U); size *= 2U) {
		auto grown = ca::reallocate(heap, current, size, 8U);
		if (!grown || grown->size() < size)
			return 2;
		for (ca::usz i = 0U; i < 20U; ++i)
			if (static_cast<ca::u8*>(grown->data())[i] != static_cast<ca::u8>(i))
				return 3;
		static_cast<ca::u8*>(grown->data())[size - 1U] = 7U;
		current = *grown;
	}
	auto shrunk = ca::reallocate(heap, current, 100U, 8U);
	if (!shrunk || static_cast<ca::u8*>(shrunk->data())[19] != 19U)
		return 4;
	auto same = ca::expand(heap, *shrunk, 110U, 8U);
	if (!same || same->data() != shrunk->data() || same->size() != 112U)
		return 5;
	if (ca::expand(heap, *same, 200U, 8U))
		return 6;
	heap.deallocate(*same, 8U);
	return 0;
}
