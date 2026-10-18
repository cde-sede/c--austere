#ifndef ca_system_syscall_impl_enter_hpp
#define ca_system_syscall_impl_enter_hpp

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/isz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYSCALL ------------------------------------------------------------

	namespace syscall {


		// -- ENTER ----------------------------------------------------------

		/* @DOC enter(number, args...) makes syscall number with exactly the arguments given, 0 to 6,
		   and returns the raw result: -errno on failure (-4095..-1). On linux x86_64, aarch64 and
		   riscv64 it is the trap instruction inlined, one overload per argument count, each
		   binding only its own registers (register variables, so the compiler loads them in
		   place). Memory is clobbered: the kernel reads and writes through pointers. Elsewhere a
		   call into libc's syscall(2), or -errc::not_supported (macOS). Perf (x86_64, getppid,
		   -O2, best of 5x5M): 117.7 ns a call with clang and g++, against 118.7 / 119.6 ns for
		   libc's syscall(2): the kernel round trip dominates; inlining saves the call, errno and
		   the setup of unused registers. */
		namespace impl {

#if ca_platform(linux) && (ca_architecture(x86_64) || ca_architecture(aarch64) || ca_architecture(riscv64))

			/* @DOC Whether enter is the trap instruction inlined, rather than a call into libc. */
			inline constexpr bool inline_entry = true;

#	if ca_architecture(x86_64)

			[[nodiscard]] inline auto enter(const ca::isz number) noexcept -> ca::isz {

				register ca::isz result asm("rax") = number;
				asm volatile("syscall" : "+r"(result) : : "rcx", "r11", "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a) noexcept -> ca::isz {

				register ca::isz result asm("rax") = number;
				register ca::isz r0 asm("rdi") = a;
				asm volatile("syscall" : "+r"(result) : "r"(r0) : "rcx", "r11", "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b) noexcept -> ca::isz {

				register ca::isz result asm("rax") = number;
				register ca::isz r0 asm("rdi") = a;
				register ca::isz r1 asm("rsi") = b;
				asm volatile("syscall" : "+r"(result) : "r"(r0), "r"(r1) : "rcx", "r11", "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b, const ca::isz c) noexcept -> ca::isz {

				register ca::isz result asm("rax") = number;
				register ca::isz r0 asm("rdi") = a;
				register ca::isz r1 asm("rsi") = b;
				register ca::isz r2 asm("rdx") = c;
				asm volatile("syscall" : "+r"(result) : "r"(r0), "r"(r1), "r"(r2) : "rcx", "r11", "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b, const ca::isz c, const ca::isz d) noexcept -> ca::isz {

				register ca::isz result asm("rax") = number;
				register ca::isz r0 asm("rdi") = a;
				register ca::isz r1 asm("rsi") = b;
				register ca::isz r2 asm("rdx") = c;
				register ca::isz r3 asm("r10") = d;
				asm volatile("syscall" : "+r"(result) : "r"(r0), "r"(r1), "r"(r2), "r"(r3) : "rcx", "r11", "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b, const ca::isz c, const ca::isz d, const ca::isz e) noexcept -> ca::isz {

				register ca::isz result asm("rax") = number;
				register ca::isz r0 asm("rdi") = a;
				register ca::isz r1 asm("rsi") = b;
				register ca::isz r2 asm("rdx") = c;
				register ca::isz r3 asm("r10") = d;
				register ca::isz r4 asm("r8") = e;
				asm volatile("syscall" : "+r"(result) : "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4) : "rcx", "r11", "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b, const ca::isz c, const ca::isz d, const ca::isz e, const ca::isz f) noexcept -> ca::isz {

				register ca::isz result asm("rax") = number;
				register ca::isz r0 asm("rdi") = a;
				register ca::isz r1 asm("rsi") = b;
				register ca::isz r2 asm("rdx") = c;
				register ca::isz r3 asm("r10") = d;
				register ca::isz r4 asm("r8") = e;
				register ca::isz r5 asm("r9") = f;
				asm volatile("syscall" : "+r"(result) : "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5) : "rcx", "r11", "memory");
				return result;
			}

#	elif ca_architecture(aarch64)

			[[nodiscard]] inline auto enter(const ca::isz number) noexcept -> ca::isz {

				register ca::isz call asm("x8") = number;
				register ca::isz result asm("x0");
				asm volatile("svc #0" : "=r"(result) : "r"(call) : "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a) noexcept -> ca::isz {

				register ca::isz call asm("x8") = number;
				register ca::isz result asm("x0") = a;
				asm volatile("svc #0" : "+r"(result) : "r"(call) : "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b) noexcept -> ca::isz {

				register ca::isz call asm("x8") = number;
				register ca::isz result asm("x0") = a;
				register ca::isz r1 asm("x1") = b;
				asm volatile("svc #0" : "+r"(result) : "r"(call), "r"(r1) : "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b, const ca::isz c) noexcept -> ca::isz {

				register ca::isz call asm("x8") = number;
				register ca::isz result asm("x0") = a;
				register ca::isz r1 asm("x1") = b;
				register ca::isz r2 asm("x2") = c;
				asm volatile("svc #0" : "+r"(result) : "r"(call), "r"(r1), "r"(r2) : "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b, const ca::isz c, const ca::isz d) noexcept -> ca::isz {

				register ca::isz call asm("x8") = number;
				register ca::isz result asm("x0") = a;
				register ca::isz r1 asm("x1") = b;
				register ca::isz r2 asm("x2") = c;
				register ca::isz r3 asm("x3") = d;
				asm volatile("svc #0" : "+r"(result) : "r"(call), "r"(r1), "r"(r2), "r"(r3) : "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b, const ca::isz c, const ca::isz d, const ca::isz e) noexcept -> ca::isz {

				register ca::isz call asm("x8") = number;
				register ca::isz result asm("x0") = a;
				register ca::isz r1 asm("x1") = b;
				register ca::isz r2 asm("x2") = c;
				register ca::isz r3 asm("x3") = d;
				register ca::isz r4 asm("x4") = e;
				asm volatile("svc #0" : "+r"(result) : "r"(call), "r"(r1), "r"(r2), "r"(r3), "r"(r4) : "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b, const ca::isz c, const ca::isz d, const ca::isz e, const ca::isz f) noexcept -> ca::isz {

				register ca::isz call asm("x8") = number;
				register ca::isz result asm("x0") = a;
				register ca::isz r1 asm("x1") = b;
				register ca::isz r2 asm("x2") = c;
				register ca::isz r3 asm("x3") = d;
				register ca::isz r4 asm("x4") = e;
				register ca::isz r5 asm("x5") = f;
				asm volatile("svc #0" : "+r"(result) : "r"(call), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5) : "memory");
				return result;
			}

#	else

			[[nodiscard]] inline auto enter(const ca::isz number) noexcept -> ca::isz {

				register ca::isz call asm("a7") = number;
				register ca::isz result asm("a0");
				asm volatile("ecall" : "=r"(result) : "r"(call) : "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a) noexcept -> ca::isz {

				register ca::isz call asm("a7") = number;
				register ca::isz result asm("a0") = a;
				asm volatile("ecall" : "+r"(result) : "r"(call) : "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b) noexcept -> ca::isz {

				register ca::isz call asm("a7") = number;
				register ca::isz result asm("a0") = a;
				register ca::isz r1 asm("a1") = b;
				asm volatile("ecall" : "+r"(result) : "r"(call), "r"(r1) : "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b, const ca::isz c) noexcept -> ca::isz {

				register ca::isz call asm("a7") = number;
				register ca::isz result asm("a0") = a;
				register ca::isz r1 asm("a1") = b;
				register ca::isz r2 asm("a2") = c;
				asm volatile("ecall" : "+r"(result) : "r"(call), "r"(r1), "r"(r2) : "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b, const ca::isz c, const ca::isz d) noexcept -> ca::isz {

				register ca::isz call asm("a7") = number;
				register ca::isz result asm("a0") = a;
				register ca::isz r1 asm("a1") = b;
				register ca::isz r2 asm("a2") = c;
				register ca::isz r3 asm("a3") = d;
				asm volatile("ecall" : "+r"(result) : "r"(call), "r"(r1), "r"(r2), "r"(r3) : "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b, const ca::isz c, const ca::isz d, const ca::isz e) noexcept -> ca::isz {

				register ca::isz call asm("a7") = number;
				register ca::isz result asm("a0") = a;
				register ca::isz r1 asm("a1") = b;
				register ca::isz r2 asm("a2") = c;
				register ca::isz r3 asm("a3") = d;
				register ca::isz r4 asm("a4") = e;
				asm volatile("ecall" : "+r"(result) : "r"(call), "r"(r1), "r"(r2), "r"(r3), "r"(r4) : "memory");
				return result;
			}

			[[nodiscard]] inline auto enter(const ca::isz number, const ca::isz a, const ca::isz b, const ca::isz c, const ca::isz d, const ca::isz e, const ca::isz f) noexcept -> ca::isz {

				register ca::isz call asm("a7") = number;
				register ca::isz result asm("a0") = a;
				register ca::isz r1 asm("a1") = b;
				register ca::isz r2 asm("a2") = c;
				register ca::isz r3 asm("a3") = d;
				register ca::isz r4 asm("a4") = e;
				register ca::isz r5 asm("a5") = f;
				asm volatile("ecall" : "+r"(result) : "r"(call), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5) : "memory");
				return result;
			}

#	endif

#else

			inline constexpr bool inline_entry = false;

			[[nodiscard]] auto enter_libc(ca::isz number, ca::isz a, ca::isz b, ca::isz c, ca::isz d, ca::isz e, ca::isz f) noexcept -> ca::isz;

			template <typename... As>
			[[nodiscard]] inline auto enter(const ca::isz number, const As... arguments) noexcept -> ca::isz {

				const ca::isz words[6]{arguments...};
				return ca::syscall::impl::enter_libc(number, words[0], words[1], words[2], words[3], words[4], words[5]);
			}

#endif

		} // namespace impl

	} // namespace syscall

} // namespace ca

#endif // ca_system_syscall_impl_enter_hpp
