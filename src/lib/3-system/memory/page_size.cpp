#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/memory/page_size.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::page_size(void) noexcept -> ca::usz {

#if ca_platform(linux) && ca_architecture(x86_64)
	return 4096U;
#else
	return static_cast<ca::usz>(::sysconf(_SC_PAGESIZE));
#endif
}
