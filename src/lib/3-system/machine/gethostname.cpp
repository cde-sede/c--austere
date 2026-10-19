#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/machine/gethostname.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/utsname.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::gethostname(const ca::span<char> buffer) noexcept -> ca::result<ca::usz> {

#if ca_platform(linux) && ca_architecture(x86_64)
	utsname raw{};
	const auto done = ca::syscall::uname(&raw);
	if (!done) [[unlikely]]
		return done.propagate();
	for (ca::usz length = 0U; length < buffer.size(); ++length) {
		buffer[length] = raw.nodename[length];
		if (raw.nodename[length] == '\0')
			return length;
	}
	return ca::failure{ca::errc::out_of_range};
#else
	if (::gethostname(buffer.data(), buffer.size()) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	for (ca::usz length = 0U; length < buffer.size(); ++length) {
		if (buffer[length] == '\0')
			return length;
	}
	return ca::failure{ca::errc::out_of_range};
#endif
}
