/* Process entry for programs linked without libc (linux x86-64, -static or -static-pie):
   _start, then the program's own relocations (static PIE), the main thread's TLS, the init
   arrays, main and exit.

   src/runtime/ is not part of libca.a: it builds libca_runtime.a, linked only by freestanding
   programs, because it defines the symbols libc's startup files and libc itself define (_start,
   exit, __cxa_atexit, __dso_handle, ...); in libca.a the linker would pull them into every
   program before reaching libc. Link line:

       c++ -nostdlib -static -no-pie prog.o -Wl,--start-group libca.a libca_runtime.a
           -Wl,--end-group -Wl,-u,_start

   1. _start (asm): the kernel leaves argc, argv, envp and the auxiliary vector on the stack;
      rdi = that stack pointer, the stack aligned to 16, then start(), which never returns.
   2. TLS (variant II): the PT_TLS image copied below the thread pointer at the offset the
      linker gave variables (p_memsz rounded up to p_align, not to the pointer's own 64-byte
      alignment), its tail zeroed (fresh pages), the
      thread control block above it (self pointer at %fs:0, stack canary at %fs:0x28 from
      AT_RANDOM), then arch_prctl(ARCH_SET_FS). Until then nothing may touch %fs: no
      thread_local, no ca::syscall call that fails (last_error is thread_local), no stack
      protector (no_stack_protector).
   3. preinit_array and init_array, each called (argc, argv, envp) as glibc does, so ca's own
      capture of the environment (3-system/environment) works in both kinds of programs.
   4. exit(main(argc, argv, envp)).
*/

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/isz.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "3-system/syscall/impl/enter.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <elf.h>

#if ca_platform(linux) && ca_architecture(x86_64)

/* The program's main, by its symbol: main may not be declared extern "C". */
auto program_main(int argc, char** argv, char** envp) -> int asm("main");

extern "C" [[noreturn]] auto exit(int status) noexcept -> void;

extern "C" char** environ;

using init_function = void (*)(int, char**, char**);

extern "C" [[gnu::visibility("hidden")]] init_function __preinit_array_start[];
extern "C" [[gnu::visibility("hidden")]] init_function __preinit_array_end[];
extern "C" [[gnu::visibility("hidden")]] init_function __init_array_start[];
extern "C" [[gnu::visibility("hidden")]] init_function __init_array_end[];

/* The dynamic section, only in a static PIE (weak: null in a static non-PIE program), and the
   ELF header, at the load address in a PIE. */
extern "C" [[gnu::visibility("hidden"), gnu::weak]] const Elf64_Dyn _DYNAMIC[];
extern "C" [[gnu::visibility("hidden")]] const Elf64_Ehdr __ehdr_start;

static constexpr ca::usz CONTROL_BLOCK = 256U;
static constexpr ca::isz ARCH_SET_FS = 0x1002;

/* Raw syscall, no error handling: TLS may not exist yet. */
template <typename... As>
[[gnu::always_inline]] static inline auto raw(const ca::isz number, const As... arguments) noexcept -> ca::isz {
	return ca::syscall::impl::enter(number, ca::syscall::impl::word(arguments)...);
}

[[gnu::no_stack_protector]] static auto auxiliary(const ca::u64* entry, const ca::u64 type) noexcept -> ca::u64 {

	for (; entry[0] != AT_NULL; entry += 2) {
		if (entry[0] == type)
			return entry[1];
	}
	return 0U;
}

[[gnu::no_stack_protector]] static auto copy(char* to, const char* from, ca::usz count) noexcept -> void {
	for (; count != 0U; --count)
		*to++ = *from++;
	asm("" : : "r"(to) : "memory");
}

/* A static PIE relocates itself before reading any global: R_X86_64_RELATIVE entries of DT_RELA
   (the only kind a static link leaves), and DT_RELR's packed ones (-z pack-relative-relocs).
   Locals and PC-relative addresses only, nothing else is in place yet: _DYNAMIC by lea in asm,
   since the compiler reads a weak symbol through the GOT, itself not yet relocated. */
[[gnu::no_stack_protector]] static auto relocate(void) noexcept -> void {

	const Elf64_Dyn* dynamic;
	asm(".weak _DYNAMIC\n\tleaq _DYNAMIC(%%rip), %0" : "=r"(dynamic));
	if (dynamic == nullptr)
		return;
	const ca::u64 base = reinterpret_cast<ca::u64>(&__ehdr_start);
	const Elf64_Rela* rela = nullptr;
	ca::usz rela_size = 0U;
	const ca::u64* relr = nullptr;
	ca::usz relr_size = 0U;
	for (; dynamic->d_tag != DT_NULL; ++dynamic) {
		if (dynamic->d_tag == DT_RELA)
			rela = reinterpret_cast<const Elf64_Rela*>(base + dynamic->d_un.d_ptr);
		else if (dynamic->d_tag == DT_RELASZ)
			rela_size = dynamic->d_un.d_val;
		else if (dynamic->d_tag == 36)
			relr = reinterpret_cast<const ca::u64*>(base + dynamic->d_un.d_ptr);
		else if (dynamic->d_tag == 35)
			relr_size = dynamic->d_un.d_val;
	}
	for (ca::usz i = 0U; rela != nullptr && i < rela_size / sizeof(Elf64_Rela); ++i) {
		if (ELF64_R_TYPE(rela[i].r_info) == R_X86_64_RELATIVE)
			*reinterpret_cast<ca::u64*>(base + rela[i].r_offset) = base + static_cast<ca::u64>(rela[i].r_addend);
	}
	ca::u64* where = nullptr;
	for (ca::usz i = 0U; relr != nullptr && i < relr_size / sizeof(ca::u64); ++i) {
		ca::u64 entry = relr[i];
		if ((entry & 1U) == 0U) {
			where = reinterpret_cast<ca::u64*>(base + entry);
			*where++ += base;
			continue;
		}
		for (ca::usz bit = 0U; (entry >>= 1U) != 0U; ++bit) {
			if ((entry & 1U) != 0U)
				where[bit] += base;
		}
		where += 63;
	}
}

/* The main thread's TLS block and control block, then %fs. */
[[gnu::no_stack_protector]] static auto set_up_thread(const ca::u64* const auxv) noexcept -> void {

	const Elf64_Phdr* const segments = reinterpret_cast<const Elf64_Phdr*>(auxiliary(auxv, AT_PHDR));
	const ca::usz count = auxiliary(auxv, AT_PHNUM);
	ca::u64 bias = 0U;
	bool placed = false;
	const Elf64_Phdr* tls = nullptr;
	for (ca::usz i = 0U; i < count; ++i) {
		if (segments[i].p_type == PT_PHDR) {
			  bias = reinterpret_cast<ca::u64>(segments) - segments[i].p_vaddr;
			placed = true;
		} else if (segments[i].p_type == PT_TLS) {
			tls = segments + i;
		}
	}
	for (ca::usz i = 0U; i < count && !placed; ++i) {
		if (segments[i].p_type == PT_LOAD && segments[i].p_offset == 0U) {
			  bias = reinterpret_cast<ca::u64>(segments) - __ehdr_start.e_phoff - segments[i].p_vaddr;
			placed = true;
		}
	}
	ca::usz align = 64U;
	ca::usz size = 0U;
	if (tls != nullptr) {
		const ca::usz own = tls->p_align > 1U ? tls->p_align : 1U;
		if (own > align)
			align = own;
		size = (tls->p_memsz + own - 1U) & ~(own - 1U);
	}
	const ca::usz length = (size + CONTROL_BLOCK + align + 4095U) & ~ca::usz{4095U};
	const ca::isz block = raw(ca::syscall::number::mmap, 0, length, 3, 0x22, -1, 0);
	if (block < 0 && block > -4096) [[unlikely]]
		raw(ca::syscall::number::exit_group, 127);
	const ca::u64 aligned = (static_cast<ca::u64>(block) + size + align - 1U) & ~static_cast<ca::u64>(align - 1U);
	char* const pointer = reinterpret_cast<char*>(aligned);
	if (tls != nullptr)
		copy(pointer - size, reinterpret_cast<const char*>(bias + tls->p_vaddr), tls->p_filesz);
	reinterpret_cast<ca::u64*>(pointer)[0] = aligned;
	const ca::u8* const random = reinterpret_cast<const ca::u8*>(auxiliary(auxv, AT_RANDOM));
	ca::u64 canary = 0U;
	if (random != nullptr)
		copy(reinterpret_cast<char*>(&canary), reinterpret_cast<const char*>(random), sizeof(canary));
	reinterpret_cast<ca::u64*>(pointer)[5] = canary & ~ca::u64{0xff};
	raw(ca::syscall::number::arch_prctl, ARCH_SET_FS, aligned);
}

extern "C" [[noreturn, gnu::no_stack_protector, gnu::used]] auto ca_runtime_start(ca::u64* const stack) noexcept -> void { // ns-leak: ignore

	relocate();
	const int argc = static_cast<int>(stack[0]);
	char** const argv = reinterpret_cast<char**>(stack + 1);
	char** const envp = argv + argc + 1;
	char** end = envp;
	while (*end != nullptr)
		++end;
	environ = envp;
	set_up_thread(reinterpret_cast<const ca::u64*>(end + 1));
	for (init_function* f = __preinit_array_start; f != __preinit_array_end; ++f)
		(*f)(argc, argv, envp);
	for (init_function* f = __init_array_start; f != __init_array_end; ++f)
		(*f)(argc, argv, envp);
	exit(program_main(argc, argv, envp));
}

asm(".text\n" // norm: ignore
	".globl _start\n"
	".type _start, @function\n"
	"_start:\n"
	"\txorl %ebp, %ebp\n"
	"\tmovq %rsp, %rdi\n"
	"\tandq $-16, %rsp\n"
	"\tcall ca_runtime_start\n"
	"\thlt\n");

#endif
