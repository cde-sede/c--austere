#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/process/impl/clone_onto.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

extern "C" auto ca_sys_clone_onto(ca::isz flags, void* stack, ca::i32* ptid, ca::i32* ctid, ca::u64 tls, int (*entry)(void*), void* argument) noexcept -> ca::isz; // ns-leak: ignore

/* The seventh argument is on the caller's stack, the kernel takes the fourth in r10. */
asm(".text\n" // norm: ignore
	".p2align 4\n"
	"ca_sys_clone_onto:\n"
	"\tmovq 8(%rsp), %rax\n"
	"\tsubq $16, %rsi\n"
	"\tmovq %rax, 8(%rsi)\n"
	"\tmovq %r9, (%rsi)\n"
	"\tmovq %rcx, %r10\n"
	"\tmovl $56, %eax\n"
	"\tsyscall\n"
	"\ttestq %rax, %rax\n"
	"\tjnz 1f\n"
	"\txorl %ebp, %ebp\n"
	"\tpopq %rax\n"
	"\tpopq %rdi\n"
	"\tcall *%rax\n"
	"\tmovl %eax, %edi\n"
	"\tmovl $60, %eax\n"
	"\tsyscall\n"
	"\thlt\n"
	"1:\n"
	"\tret\n");

[[nodiscard]] auto ca::sys::impl::clone_onto(const ca::isz flags, void* const stack, ca::i32* const ptid, ca::i32* const ctid, const ca::u64 tls, int (* const entry)(void*), void* const argument) noexcept -> ca::result<ca::i32> {

	const ca::isz child = ca_sys_clone_onto(flags, stack, ptid, ctid, tls, entry, argument);
	if (child < 0) [[unlikely]]
		return ca::failure{ca::error{static_cast<int>(-child)}};
	return static_cast<ca::i32>(child);
}

#endif
