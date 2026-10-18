// expect: exit 0
#include "3-system/memory/move_paths.hpp"

auto main(void) -> int {
	char from[512];
	char to[512];
	for (int i = 0; i < 512; ++i) {
		from[i] = static_cast<char>(i);
		to[i] = 7;
	}
	volatile ca::usz count = 40U;
	auto refused = ca::move_paths::bytes_16_to_32(to, from, count);
	if (refused || refused.error() != ca::errc::out_of_range || to[0] != 7 || to[39] != 7)
		return 1;
	if (ca::move_paths::bytes_0_to_15(to, from, count) || ca::move_paths::bytes_64_to_128(to, from, count))
		return 2;
	if (ca::move_paths::forward(to, from, 128U) || ca::move_paths::large(to, from, count))
		return 3;
	if (ca::move_paths::avx2::bytes_128_to_256(to, from, count) || ca::move_paths::avx2::stream(to, from, 256U))
		return 4;
	if (!ca::move_paths::bytes_32_to_64(to, from, count) || to[39] != 39 || to[40] != 7)
		return 5;
	return 0;
}
