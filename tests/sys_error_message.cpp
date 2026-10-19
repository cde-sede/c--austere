// expect: exit 0
// link: libc
#include "1-primitives/memory/string_length.hpp"
#include "3-system/error/error_message.hpp"

extern "C" auto strerrordesc_np(int code) noexcept -> const char*;

auto main(void) -> int {
	for (int code = -5; code < 300; ++code) {
		const char* const libc = strerrordesc_np(code);
		const ca::sv::str expected = libc != nullptr ? ca::sv::str{libc, ca::string_length(libc)} : ca::sv::str{"Unknown error"};
		if (ca::sys::error_message(ca::error{code}) != expected)
			return 1;
	}
	return 0;
}
