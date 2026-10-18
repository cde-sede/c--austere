// expect: error no matching function|too many arguments
#include "3-system/syscall/syscalls.hpp"

auto main(void) -> int {
	return ca::syscall::getpid(1) ? 0 : 1;
}
