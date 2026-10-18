#include "3-system/process/exit_immediately.hpp"

#include <unistd.h>


[[noreturn]] auto ca::sys::exit_immediately(const int status) noexcept -> void {
	::_exit(status);
}
