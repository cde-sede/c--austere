#include "1-primitives/integers/i32.hpp"
#include "3-system/process/getpid.hpp"

#include <sys/types.h>
#include <unistd.h>


static_assert(sizeof(pid_t) == sizeof(ca::i32));

[[nodiscard]] auto ca::sys::getpid(void) noexcept -> ca::i32 {
	return static_cast<ca::i32>(::getpid());
}
