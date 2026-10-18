// Benchmarks for ca::copy_bytes and ca::move_bytes at run-time sizes; -DUSE_STD runs the same
// cases on glibc's memcpy and memmove (the ifunc-selected variant, called through the PLT).
// Fixed sizes on hot buffers, misaligned pointers, overlapping moves both ways, and a mix of
// small sizes drawn so the branch predictor cannot learn one.

#include "measure.hpp"

static const size_t TOTAL = size_t{1} << 30U;
static const size_t BUFFER = size_t{64} << 20U;
static const int MIX = 4096;

alignas(64) static char source[BUFFER + 4096];
alignas(64) static char target[BUFFER + 4096];

static unsigned mix_sizes[MIX];
static unsigned mix_offsets[MIX];

static volatile size_t opaque;

#ifdef USE_STD
static auto copy(void* const to, const void* const from, const size_t count) -> void {
	memcpy(to, from, count);
}

static auto move(void* const to, const void* const from, const size_t count) -> void {
	memmove(to, from, count);
}
#else
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/move_bytes.hpp"

static auto copy(void* const to, const void* const from, const size_t count) -> void {
	ca::copy_bytes(to, from, count);
}

static auto move(void* const to, const void* const from, const size_t count) -> void {
	ca::move_bytes(to, from, count);
}
#endif


// -- CASES ------------------------------------------------------------------

NI static auto fixed(const size_t size, const size_t from, const size_t to) -> void {
	const size_t count = opaque + size;
	const size_t rounds = TOTAL / (size < 64U ? 64U : size) / (size >= BUFFER / 4U ? 4U : 1U);
	for (size_t i = 0; i < rounds; ++i)
		copy(target + to, source + from, count);
	sink = static_cast<unsigned char>(target[to + count - 1U]);
}

NI static auto streaming(const size_t size) -> void {
	const size_t count = opaque + size;
	const size_t slots = BUFFER / size;
	const size_t rounds = TOTAL / 4U / size;
	for (size_t i = 0; i < rounds; ++i)
		copy(target + i % slots * size, source + (i + slots / 2U) % slots * size, count);
	sink = static_cast<unsigned char>(target[count - 1U]);
}

NI static auto overlapping(const size_t size, const size_t from, const size_t to) -> void {
	const size_t count = opaque + size;
	const size_t rounds = TOTAL / 4U / (size < 64U ? 64U : size);
	for (size_t i = 0; i < rounds; ++i)
		move(source + to, source + from, count);
	sink = static_cast<unsigned char>(source[to + count - 1U]);
}

NI static auto mixed(void) -> void {
	const size_t rounds = 4096U * 16U;
	size_t sum = opaque;
	for (size_t round = 0; round < rounds; ++round) {
		for (int i = 0; i < MIX; i += 16) {
			const unsigned offset = mix_offsets[(static_cast<size_t>(i) + round) % MIX];
			copy(target + offset, source + (offset ^ 1984U), mix_sizes[i + static_cast<int>(round % 16U)]);
		}
		sum += static_cast<unsigned char>(target[round % 4096U]);
	}
	sink = sum;
}


// -- MAIN -------------------------------------------------------------------

static auto mix_size(unsigned seed) -> unsigned {
	seed = seed * 2654435761U ^ seed >> 13U;
	const unsigned bucket = seed % 100U;
	if (bucket < 50U)
		return seed >> 8U & 31U;
	if (bucket < 80U)
		return 32U + (seed >> 8U & 95U);
	if (bucket < 95U)
		return 128U + (seed >> 8U & 383U);
	return 512U + (seed >> 8U & 1535U);
}

auto main(void) -> int {
	for (size_t i = 0; i < sizeof source; ++i)
		source[i] = static_cast<char>('a' + i % 26U);
	for (unsigned i = 0; i < MIX; ++i) {
		mix_sizes[i] = mix_size(i);
		mix_offsets[i] = (i * 2246822519U >> 7U) % 2048U;
	}
	if (!open_counters())
		return 1;
	static const size_t sizes[] = {1, 7, 16, 33, 64, 100, 256, 300, 1024, 4096, 16384, 65536, 1U << 20U};
	static const char* const names[] = {"1", "7", "16", "33", "64", "100", "256", "300", "1K", "4K", "16K", "64K", "1M"};
	char name[64];
	for (size_t i = 0; i < sizeof sizes / sizeof *sizes; ++i) {
		snprintf(name, sizeof name, "copy %s", names[i]);
		bench(name, [=] { fixed(sizes[i], 0U, 0U); });
	}
	bench("copy 100 +1/+3", [] { fixed(100U, 1U, 3U); });
	bench("copy 300 +1/+3", [] { fixed(300U, 1U, 3U); });
	bench("copy 4K +1/+3", [] { fixed(4096U, 1U, 3U); });
	bench("copy 64K +1/+3", [] { fixed(65536U, 1U, 3U); });
	bench("copy 1M +1/+3", [] { fixed(1U << 20U, 1U, 3U); });
	bench("stream 4K over 64M", [] { streaming(4096U); });
	bench("stream 1M over 64M", [] { streaming(1U << 20U); });
	bench("stream 16M over 64M", [] { streaming(16U << 20U); });
	bench("move 100 fwd -7", [] { overlapping(100U, 7U, 0U); });
	bench("move 100 bwd +7", [] { overlapping(100U, 0U, 7U); });
	bench("move 4K fwd -100", [] { overlapping(4096U, 100U, 0U); });
	bench("move 4K bwd +100", [] { overlapping(4096U, 0U, 100U); });
	bench("move 64K bwd +4K", [] { overlapping(65536U, 0U, 4096U); });
	bench("mix 0-2K", mixed);
	return 0;
}
