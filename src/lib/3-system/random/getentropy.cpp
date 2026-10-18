#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/byte.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/random/getentropy.hpp"

#include <unistd.h>


#if ca_platform(macos)
#	include <sys/random.h>
#endif


[[nodiscard]] auto ca::sys::getentropy(const ca::span<ca::byte> buffer) noexcept -> ca::result<void> {

	ca::byte* at = buffer.data();
	ca::usz left = buffer.size();
	while (left != 0U) {
		const ca::usz chunk = left < 256U ? left : 256U;
		if (::getentropy(at, chunk) == -1) [[unlikely]]
			return ca::failure{ca::sys::impl::libc_error()};
		  at += chunk;
		left -= chunk;
	}
	return ca::result<void>{};
}
