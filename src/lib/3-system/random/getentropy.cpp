#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/byte.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/random/getentropy.hpp"
#include "3-system/syscall/restarting.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <unistd.h>


#if ca_platform(macos)
#	include <sys/random.h>
#endif


[[nodiscard]] auto ca::sys::getentropy(const ca::span<ca::byte> buffer) noexcept -> ca::result<void> {

	ca::byte* at = buffer.data();
	ca::usz left = buffer.size();
	while (left != 0U) {
		const ca::usz chunk = left < 256U ? left : 256U;
#if ca_platform(linux) && ca_architecture(x86_64)
		const auto filled = ca::syscall::restarting([at, chunk] {
			return ca::syscall::getrandom(at, chunk, 0);
		});
		if (!filled) [[unlikely]]
			return filled.propagate();
		const ca::usz done = static_cast<ca::usz>(*filled);
#else
		if (::getentropy(at, chunk) == -1) [[unlikely]]
			return ca::failure{ca::sys::impl::libc_error()};
		const ca::usz done = chunk;
#endif
		  at += done;
		left -= done;
	}
	return ca::result<void>{};
}
