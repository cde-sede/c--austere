#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/syscall/impl/enter.hpp"

#if ca_platform(linux) && !ca_architecture(x86_64) && !ca_architecture(aarch64) && !ca_architecture(riscv64)
#	include <errno.h>
#	include <unistd.h>


[[nodiscard]] auto ca::syscall::impl::enter_libc(const ca::isz number, const ca::isz a, const ca::isz b, const ca::isz c, const ca::isz d, const ca::isz e, const ca::isz f) noexcept -> ca::isz {

	const long done = ::syscall(number, a, b, c, d, e, f);
	return done == -1 ? -static_cast<ca::isz>(errno) : static_cast<ca::isz>(done);
}

#elif !ca_platform(linux)


[[nodiscard]] auto ca::syscall::impl::enter_libc(const ca::isz, const ca::isz, const ca::isz, const ca::isz, const ca::isz, const ca::isz, const ca::isz) noexcept -> ca::isz {
	return -static_cast<ca::isz>(ca::errc::not_supported.code());
}

#endif
