// expect: error is_argument|constraints not satisfied
#include "3-system/syscall/syscalls.hpp"

struct pair {
	int a;
	int b;
};

auto main(void) -> int {
	return ca::syscall::write(1, pair{1, 2}, 8) ? 0 : 1;
}
