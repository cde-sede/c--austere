#include "3-system/process/getegid.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::getegid(void) noexcept -> ca::u32 {
	return static_cast<ca::u32>(::getegid());
}
