// expect: exit 3
#include "3-system/syscall/syscalls.hpp"

auto main(void) -> int {
	ca::syscall::exit_group(3);
}
