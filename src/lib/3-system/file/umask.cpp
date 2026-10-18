#include "3-system/file/umask.hpp"

#include <sys/stat.h>


[[nodiscard]] auto ca::sys::umask(const ca::u32 mask) noexcept -> ca::u32 {
	return static_cast<ca::u32>(::umask(static_cast<mode_t>(mask)));
}
