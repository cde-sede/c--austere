#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/execve.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::execve(const char* const path, char* const* const argv, char* const* const envp) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::execve(path, argv, envp).transform([](ca::isz) {});
#else
	::execve(path, argv, envp);
	return ca::failure{ca::sys::impl::libc_error()};
#endif
}
