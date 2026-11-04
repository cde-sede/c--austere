// expect: exit 0
// link: both
#include "1-primitives/integers/byte.hpp"
#include "3-system/memory/mmap_anonymous.hpp"
#include "3-system/memory/munmap.hpp"
#include "3-system/memory/protection.hpp"
#include "3-system/process/impl/clone_onto.hpp"
#include "3-system/syscall/syscalls.hpp"

static constexpr ca::isz CLONE_VM = 0x100;
static constexpr ca::isz CLONE_VFORK = 0x4000;
static constexpr ca::isz CLONE_PARENT_SETTID = 0x100000;
static constexpr ca::isz CLONE_CHILD_SETTID = 0x1000000;
static constexpr ca::isz SIGCHLD = 17;
static constexpr ca::usz STACK = 65536U;

static auto child(void* const argument) noexcept -> int {

	int& seen = *static_cast<int*>(argument);
	seen = 7;
	return 42;
}

auto main(void) -> int {
	auto stack = ca::sys::mmap_anonymous(STACK, ca::sys::protection::read | ca::sys::protection::write);
	if (!stack)
		return 1;
	int seen = 0;
	ca::i32 parent_id = 0;
	ca::i32 child_id = 0;
	const auto started = ca::sys::impl::clone_onto(CLONE_VM | CLONE_VFORK | CLONE_PARENT_SETTID | CLONE_CHILD_SETTID | SIGCHLD, stack->data() + STACK, &parent_id, &child_id, 0U, &child, &seen);
	if (!started || *started <= 0)
		return 2;
	if (parent_id != *started || child_id != *started || seen != 7)
		return 3;
	int status = 0;
	if (!ca::syscall::wait4(*started, &status, 0, nullptr) || status != 42 << 8)
		return 4;
	const ca::isz invalid = CLONE_VM | CLONE_VFORK | 0x10000;
	if (ca::sys::impl::clone_onto(invalid, stack->data() + STACK, nullptr, nullptr, 0U, &child, &seen))
		return 5;
	return ca::sys::munmap(*stack) ? 0 : 6;
}
