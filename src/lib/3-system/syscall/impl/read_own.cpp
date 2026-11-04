#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/isz.hpp"
#include "3-system/io/io_buffer.hpp"
#include "3-system/syscall/impl/enter.hpp"
#include "3-system/syscall/impl/read_own.hpp"
#include "3-system/syscall/syscalls.hpp"


[[nodiscard]] auto ca::syscall::impl::read_own(const ca::usz address, const ca::span<ca::byte> out) noexcept -> bool {

#if ca_platform(linux) && ca_architecture(x86_64)
	const ca::sys::io_buffer local{ca::span<const ca::byte>{out.data(), out.size()}};
	const ca::sys::io_buffer remote{ca::span<const ca::byte>{reinterpret_cast<const ca::byte*>(address), out.size()}};
	const ca::isz pid = ca::syscall::impl::enter(ca::syscall::number::getpid);
	const ca::isz read = ca::syscall::impl::enter(ca::syscall::number::process_vm_readv, pid, reinterpret_cast<ca::isz>(&local), 1,
	                                              reinterpret_cast<ca::isz>(&remote), 1, 0);
	return read == static_cast<ca::isz>(out.size());
#else
	static_cast<void>(address);
	static_cast<void>(out);
	return false;
#endif
}
