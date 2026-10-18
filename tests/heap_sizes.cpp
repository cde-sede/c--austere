// expect: exit 0
#include "1-primitives/integers/u8.hpp"
#include "4-lifecycle/memory/heap.hpp"

static auto check(const ca::usz size, const ca::usz alignment) noexcept -> bool {

	const ca::heap heap{};
	auto block = heap.allocate(size, alignment);
	if (!block)
		return false;
	const ca::usz address = reinterpret_cast<ca::usz>(block->data());
	if (block->size() < size || (address & (alignment - 1U)) != 0U || block->data() == nullptr)
		return false;
	ca::u8* const bytes = static_cast<ca::u8*>(block->data());
	for (ca::usz i = 0U; i < block->size(); ++i)
		bytes[i] = static_cast<ca::u8>(i);
	for (ca::usz i = 0U; i < block->size(); ++i)
		if (bytes[i] != static_cast<ca::u8>(i))
			return false;
	heap.deallocate(block->with_size(size), alignment);
	return true;
}

auto main(void) -> int {
	for (ca::usz size = 1U; size <= 4096U; ++size)
		if (!check(size, 1U) || !check(size, 16U))
			return 1;
	for (ca::usz size = 4096U; size <= 600000U; size += 4093U)
		if (!check(size, 8U))
			return 2;
	const ca::usz alignments[] = {32U, 64U, 256U, 4096U, 65536U, 131072U, 262144U, 2097152U};
	const ca::usz sizes[] = {1U, 48U, 5000U, 131072U, 200000U};
	for (const ca::usz alignment : alignments)
		for (const ca::usz size : sizes)
			if (!check(size, alignment))
				return 3;
	auto empty = ca::heap{}.allocate(0U, 64U);
	if (!empty || empty->size() != 0U || empty->data() == nullptr)
		return 4;
	ca::heap{}.deallocate(*empty, 64U);
	return 0;
}
