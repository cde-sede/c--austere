#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/memory/mmap_anonymous.hpp"
#include "3-system/memory/protection.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <sys/mman.h>


static_assert(ca::sys::protection::none == PROT_NONE);
static_assert(ca::sys::protection::read == PROT_READ);
static_assert(ca::sys::protection::write == PROT_WRITE);
static_assert(ca::sys::protection::execute == PROT_EXEC);

[[nodiscard]] auto ca::sys::mmap_anonymous(const ca::usz size, const int protection) noexcept -> ca::result<ca::span<ca::byte>> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::mmap(nullptr, size, protection, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0).transform([size](const ca::isz address) {
		return ca::span<ca::byte>{reinterpret_cast<ca::byte*>(address), size};
	});
#else
	void* const block = ::mmap(nullptr, size, protection, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (block == MAP_FAILED) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::span<ca::byte>{static_cast<ca::byte*>(block), size};
#endif
}
