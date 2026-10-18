#include "3-system/memory/page_size.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::page_size(void) noexcept -> ca::usz {
	return static_cast<ca::usz>(::sysconf(_SC_PAGESIZE));
}
