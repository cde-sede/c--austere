#include "3-system/io/isatty.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::isatty(const int fd) noexcept -> bool {
	return ::isatty(fd) == 1;
}
