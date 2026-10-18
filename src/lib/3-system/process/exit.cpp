#include "3-system/process/exit.hpp"

#include <stdlib.h>


[[noreturn]] auto ca::sys::exit(const int status) noexcept -> void {
	::exit(status);
}
