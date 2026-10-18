// expect: error (size_t|open|was not declared|no member)
#include "3-system/file/open.hpp"
#include "3-system/memory/malloc_alignment.hpp"
#include "6-resources/fs/descriptor.hpp"

auto main(void) -> int {
	return ::open("/dev/null", 0) + static_cast<int>(sizeof(::size_t));
}
