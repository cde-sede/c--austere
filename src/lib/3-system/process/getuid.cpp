#include "1-primitives/integers/u32.hpp"
#include "3-system/process/getuid.hpp"

#include <sys/types.h>
#include <unistd.h>


static_assert(sizeof(uid_t) == sizeof(ca::u32));

[[nodiscard]] auto ca::sys::getuid(void) noexcept -> ca::u32 {
	return static_cast<ca::u32>(::getuid());
}
