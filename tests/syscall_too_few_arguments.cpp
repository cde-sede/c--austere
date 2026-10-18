// expect: error no matching function|too few arguments
#include "3-system/syscall/syscalls.hpp"

auto main(void) -> int {
	char buffer[4]{};
	return ca::syscall::read(0, buffer) ? 0 : 1;
}
