#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/io/pipe.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <fcntl.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::pipe(const int flags) noexcept -> ca::result<ca::sys::pipe_ends> {

	int ends[2]{-1, -1};
#if ca_platform(linux) && ca_architecture(x86_64)
	const auto done = ca::syscall::pipe2(ends, flags);
	if (!done) [[unlikely]]
		return done.propagate();
#elif ca_platform(linux)
	if (::pipe2(ends, flags) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
#else
	if (::pipe(ends) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	for (const int end : ends) {
		const bool failed = ((flags & O_CLOEXEC) != 0 && ::fcntl(end, F_SETFD, FD_CLOEXEC) == -1)
			|| ((flags & O_NONBLOCK) != 0 && ::fcntl(end, F_SETFL, ::fcntl(end, F_GETFL) | O_NONBLOCK) == -1);
		if (failed) [[unlikely]] {
			const ca::error error = ca::sys::impl::libc_error();
			::close(ends[0]);
			::close(ends[1]);
			return ca::failure{error};
		}
	}
#endif
	return ca::sys::pipe_ends{ends[0], ends[1]};
}
