#include "3-system/memory/free.hpp"

#include <stdlib.h>


auto ca::sys::free(void* const block) noexcept -> void {
	::free(block);
}
