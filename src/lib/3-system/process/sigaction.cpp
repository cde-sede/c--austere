#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/sigaction.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <signal.h>


static_assert(ca::sys::sigaction_flags::on_alternate_stack == SA_ONSTACK);
static_assert(ca::sys::sigaction_flags::restart == SA_RESTART);
static_assert(ca::sys::sigaction_flags::no_defer == SA_NODEFER);
static_assert(ca::sys::sigaction_flags::reset_handler == SA_RESETHAND);

#if ca_platform(linux) && ca_architecture(x86_64)

namespace ca {


	namespace sys {


		namespace impl {

			/* @DOC struct sigaction as the kernel reads it on x86-64 (struct kernel_sigaction). */
			struct kernel_action final {
					using self = ca::sys::impl::kernel_action;
					void (*_handler)(int);
					unsigned long _flags;
					void (*_restorer)(void);
					ca::u64 _mask;
			}; // struct kernel_action

		} // namespace impl

	} // namespace sys

} // namespace ca

/* Where a handler returns to: the rt_sigreturn syscall, which restores the interrupted context
   the kernel saved on the stack. Local to this object. */
asm(".text\n" // norm: ignore
	".p2align 4\n"
	"ca_signal_restorer:\n"
	"\tmovq $15, %rax\n"
	"\tsyscall\n");

extern "C" auto ca_signal_restorer(void) noexcept -> void; // ns-leak: ignore

static constexpr unsigned long RESTORER = 0x04000000UL;

[[nodiscard]] static auto install(const int signal, void (*const handler)(int), const ca::u64 flags) noexcept -> ca::result<void> {

	const ca::sys::impl::kernel_action action{handler, static_cast<unsigned long>(flags) | RESTORER, &ca_signal_restorer, 0U};
	return ca::syscall::rt_sigaction(signal, &action, nullptr, 8).transform([](ca::isz) {});
}

#else

[[nodiscard]] static auto install(const int signal, void (*const handler)(int), const ca::u64 flags) noexcept -> ca::result<void> {

	struct ::sigaction action{};
	action.sa_handler = handler;
	::sigemptyset(&action.sa_mask);
	action.sa_flags = static_cast<int>(flags);
	if (::sigaction(signal, &action, nullptr) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}

#endif

[[nodiscard]] auto ca::sys::sigaction(const int signal, void (*const handler)(int)) noexcept -> ca::result<void> {
	return install(signal, handler, ca::sys::sigaction_flags::restart);
}

[[nodiscard]] auto ca::sys::sigaction(const int signal, void (*const handler)(int), const ca::u64 flags) noexcept -> ca::result<void> {
	return install(signal, handler, flags);
}

[[nodiscard]] auto ca::sys::sigaction(const int signal, const ca::sys::signal_disposition disposition) noexcept -> ca::result<void> {
	return install(signal, disposition == ca::sys::signal_disposition::ignore ? SIG_IGN : SIG_DFL, ca::sys::sigaction_flags::restart);
}
