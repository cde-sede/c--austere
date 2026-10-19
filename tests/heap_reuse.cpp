// expect: exit 0
// link: both
#include "4-lifecycle/memory/heap.hpp"

auto main(void) -> int {

	const ca::heap heap{};
	auto first = heap.allocate(24U, 8U);
	if (!first || first->size() != 32U)
		return 1;
	heap.deallocate(*first, 8U);
	auto again = heap.allocate(32U, 8U);
	if (!again || again->data() != first->data())
		return 2;
	void* blocks[100000];
	for (void*& block : blocks) {
		auto taken = heap.allocate(100U, 8U);
		if (!taken || taken->size() != 112U)
			return 3;
		block = taken->data();
	}
	for (void* const block : blocks)
		heap.deallocate(ca::allocation{block, 100U}, 8U);
	for (void*& block : blocks) {
		auto taken = heap.allocate(100U, 8U);
		if (!taken)
			return 4;
		block = taken->data();
	}
	for (void* const block : blocks)
		heap.deallocate(ca::allocation{block, 112U}, 8U);
	heap.deallocate(*again, 32U);
	return 0;
}
