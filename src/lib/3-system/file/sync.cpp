#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/sync.hpp"
#include "3-system/syscall/restarting.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <unistd.h>


#if !(ca_platform(linux) && ca_architecture(x86_64))

static auto sync_all(void) noexcept -> void {
	::sync();
}

#endif

[[nodiscard]] auto ca::sys::sync(void) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::restarting([] {
		return ca::syscall::sync();
	}).transform([](ca::isz) {});
#else
	sync_all();
	return ca::result<void>{};
#endif
}
