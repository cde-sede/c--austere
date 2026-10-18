#include "1-primitives/integers/i32.hpp"
#include "3-system/process/getppid.hpp"

#include <sys/types.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::getppid(void) noexcept -> ca::i32 {
	return static_cast<ca::i32>(::getppid());
}
