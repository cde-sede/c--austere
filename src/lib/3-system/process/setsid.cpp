#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/setsid.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::setsid(void) noexcept -> ca::result<ca::i32> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::setsid().transform([](const ca::isz session) {
		return static_cast<ca::i32>(session);
	});
#else
	const pid_t session = ::setsid();
	if (session == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::i32>(session);
#endif
}
