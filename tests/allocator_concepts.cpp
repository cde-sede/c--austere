// expect: exit 0
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateful_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "4-lifecycle/memory/libc_allocator.hpp"
#include "4-lifecycle/memory/page_allocator.hpp"

class pointer_allocator final {
	private:
		int* _counter;
	public:
		explicit pointer_allocator(int* const counter) noexcept
		: _counter{counter} {
		}
		auto allocate(const ca::usz size, const ca::usz alignment) const noexcept -> ca::result<ca::allocation> {
			++*_counter;
			return ca::heap{}.allocate(size, alignment);
		}
		auto deallocate(const ca::allocation block, const ca::usz alignment) const noexcept -> void {
			--*_counter;
			ca::heap{}.deallocate(block, alignment);
		}
};

class lying_allocator final {
	private:
		int* _state;
	public:
		static constexpr bool STATELESS = true;
		auto allocate(ca::usz, ca::usz) const noexcept -> ca::result<ca::allocation>;
		auto deallocate(ca::allocation, ca::usz) const noexcept -> void;
};

class empty_but_stateful final {
	public:
		auto allocate(ca::usz, ca::usz) const noexcept -> ca::result<ca::allocation>;
		auto deallocate(ca::allocation, ca::usz) const noexcept -> void;
};

static_assert(ca::is_stateless_allocator<ca::heap>);
static_assert(ca::is_stateless_allocator<ca::page_allocator>);
static_assert(ca::is_stateless_allocator<ca::libc_allocator>);
static_assert(ca::is_stateful_allocator<pointer_allocator>);
static_assert(!ca::is_allocator<lying_allocator>);
static_assert(ca::is_stateful_allocator<empty_but_stateful>);
static_assert(!ca::is_allocator<int>);

auto main(void) -> int {
	int counter = 0;
	const pointer_allocator allocator{&counter};
	auto block = allocator.allocate(10U, 8U);
	if (!block || counter != 1)
		return 1;
	allocator.deallocate(*block, 8U);
	return counter == 0 ? 0 : 2;
}
