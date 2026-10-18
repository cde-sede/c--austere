// expect: exit 0
#include "4-lifecycle/memory/page_allocator.hpp"

auto main(void) -> int {

	const ca::page_allocator pages{};
	auto block = pages.allocate(5000U, 8U);
	if (!block || block->size() != 8192U || (reinterpret_cast<ca::usz>(block->data()) & 4095U) != 0U)
		return 1;
	char* const bytes = static_cast<char*>(block->data());
	if (bytes[0] != 0 || bytes[8191] != 0)
		return 2;
	bytes[100] = 42;
	auto moved = pages.reallocate(*block, ca::usz{256U} << 20U, 8U);
	if (!moved || moved->size() != (ca::usz{256U} << 20U) || static_cast<char*>(moved->data())[100] != 42)
		return 3;
	auto shrunk = pages.expand(*moved, 4096U, 8U);
	if (!shrunk || shrunk->data() != moved->data() || shrunk->size() != 4096U)
		return 4;
	pages.deallocate(*shrunk, 8U);
	auto aligned = pages.allocate(10U, ca::usz{1U} << 21U);
	if (!aligned || (reinterpret_cast<ca::usz>(aligned->data()) & ((ca::usz{1U} << 21U) - 1U)) != 0U)
		return 5;
	pages.deallocate(*aligned, ca::usz{1U} << 21U);
	auto huge = pages.allocate(~ca::usz{0U} - 10U, 8U);
	return huge ? 6 : 0;
}
