#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/memory/mremap.hpp"
#include "3-system/syscall/syscalls.hpp"

#if ca_platform(linux)
#	include <sys/mman.h>
#endif


[[nodiscard]] auto ca::sys::mremap(const ca::span<ca::byte> mapping, const ca::usz size, const ca::sys::remapping remapping) noexcept -> ca::result<ca::span<ca::byte>> {

#if ca_platform(linux)
	const int flags = remapping == ca::sys::remapping::may_move ? MREMAP_MAYMOVE : 0;
#	if ca_architecture(x86_64)
	return ca::syscall::mremap(mapping.data(), mapping.size(), size, flags, nullptr).transform([size](const ca::isz address) {
		return ca::span<ca::byte>{reinterpret_cast<ca::byte*>(address), size};
	});
#	else
	void* const moved = ::mremap(mapping.data(), mapping.size(), size, flags);
	if (moved == MAP_FAILED) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::span<ca::byte>{static_cast<ca::byte*>(moved), size};
#	endif
#else
	static_cast<void>(mapping);
	static_cast<void>(size);
	static_cast<void>(remapping);
	return ca::failure{ca::errc::not_supported};
#endif
}
