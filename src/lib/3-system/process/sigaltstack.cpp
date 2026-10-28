#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/sigaltstack.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <signal.h>


[[nodiscard]] auto ca::sys::sigaltstack(const ca::span<char> stack) noexcept -> ca::result<void> {

	stack_t wanted{};
	   wanted.ss_sp = stack.data();
	 wanted.ss_size = stack.size();
	wanted.ss_flags = stack.size() == 0U ? SS_DISABLE : 0;
#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::sigaltstack(&wanted, nullptr).transform([](ca::isz) {});
#else
	if (::sigaltstack(&wanted, nullptr) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
