#include "3-system/machine/processor_count.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::processor_count(void) noexcept -> ca::usz {

	const long count = ::sysconf(_SC_NPROCESSORS_ONLN);
	return count < 1 ? 1U : static_cast<ca::usz>(count);
}
