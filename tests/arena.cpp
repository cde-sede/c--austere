// expect: exit 0
#include "4-lifecycle/is_cleanable.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "4-lifecycle/memory/expand.hpp"
#include "4-lifecycle/memory/is_allocator_resource.hpp"
#include "4-lifecycle/memory/is_stateful_allocator.hpp"
#include "4-lifecycle/memory/reallocate.hpp"

static_assert(ca::is_allocator_resource<ca::arena>);
static_assert(ca::is_stateful_allocator<ca::arena_allocator>);
static_assert(ca::is_cleanable<ca::arena>);
static_assert(sizeof(ca::arena_allocator) == sizeof(void*));

auto main(void) -> int {

	auto made = ca::arena::make(4096U);
	if (!made)
		return 1;
	ca::arena arena = ca::move(*made);
	const ca::arena_allocator allocator = arena.allocator();
	auto first = allocator.allocate(10U, 1U);
	auto second = allocator.allocate(8U, 64U);
	if (!first || !second || (reinterpret_cast<ca::usz>(second->data()) & 63U) != 0U)
		return 2;
	if (static_cast<char*>(second->data()) < static_cast<char*>(first->data()) + 10)
		return 3;
	allocator.deallocate(*second, 64U);
	auto again = allocator.allocate(8U, 64U);
	if (!again || again->data() != second->data())
		return 4;
	auto grown = ca::expand(allocator, *again, 1000U, 64U);
	if (!grown || grown->data() != again->data())
		return 5;
	auto big = allocator.allocate(100000U, 16U);
	if (!big)
		return 6;
	static_cast<char*>(big->data())[99999] = 1;
	auto moved = ca::reallocate(allocator, *first, 20U, 1U);
	if (!moved || moved->data() == first->data())
		return 7;
	ca::arena other = ca::move(arena);
	if (arena.holds() || !other.holds())
		return 8;
	auto after_move = allocator.allocate(16U, 16U);
	if (!after_move)
		return 9;
	other.reset();
	auto after_reset = allocator.allocate(16U, 16U);
	if (!after_reset)
		return 10;
	return other.destroy() && !other.holds() ? 0 : 11;
}
