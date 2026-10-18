#include "3-system/process/geteuid.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::geteuid(void) noexcept -> ca::u32 {
	return static_cast<ca::u32>(::geteuid());
}
