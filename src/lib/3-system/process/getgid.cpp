#include "1-primitives/integers/u32.hpp"
#include "3-system/process/getgid.hpp"

#include <sys/types.h>
#include <unistd.h>


static_assert(sizeof(gid_t) == sizeof(ca::u32));

[[nodiscard]] auto ca::sys::getgid(void) noexcept -> ca::u32 {
	return static_cast<ca::u32>(::getgid());
}
