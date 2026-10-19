#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/memory/string_length.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/getcwd.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::getcwd(const ca::span<char> buffer) noexcept -> ca::result<ca::usz> {

#if ca_platform(linux) && ca_architecture(x86_64)
	const auto length = ca::syscall::getcwd(buffer.data(), buffer.size());
	if (!length) [[unlikely]]
		return length.propagate();
	if (buffer.data()[0] != '/') [[unlikely]] {
		ca::sys::set_last_error(ca::errc::no_entry);
		return ca::failure{ca::errc::no_entry};
	}
	return static_cast<ca::usz>(*length) - 1U;
#else
	if (::getcwd(buffer.data(), buffer.size()) == nullptr) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::string_length(buffer.data());
#endif
}
