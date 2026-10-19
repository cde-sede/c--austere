// Benchmarks for ca::compare_bytes (memcmp) and ca::equal_units (memcmp == 0, bcmp on clang) at
// run-time sizes; -DUSE_STD runs the same cases on glibc's memcmp (the ifunc-selected variant,
// called through the PLT). Equal ranges of fixed sizes (the whole range is read), a difference
// at a random place, misaligned pointers, and a mix of small sizes drawn so the branch predictor
// cannot learn one. The comparison is pure: fixed-size rounds hide the pointer behind an empty
// asm, or the call is hoisted out of the loop.

#include "measure.hpp"

static const size_t TOTAL = size_t{1} << 30U;
static const size_t BUFFER = size_t{16} << 20U;
static const int MIX = 4096;

alignas(64) static char left[BUFFER + 4096];
alignas(64) static char right[BUFFER + 4096];

static unsigned mix_sizes[MIX];
static unsigned mix_offsets[MIX];

static volatile size_t opaque;

#ifdef USE_STD
static auto compare(const void* const a, const void* const b, const size_t count) -> int {
	return memcmp(a, b, count);
}

static auto equal(const void* const a, const void* const b, const size_t count) -> bool {
	return memcmp(a, b, count) == 0;
}
#else
#include "1-primitives/memory/compare_bytes.hpp"
#include "1-primitives/memory/equal_units.hpp"

static auto compare(const void* const a, const void* const b, const size_t count) -> int {
	return ca::compare_bytes(static_cast<const char*>(a), static_cast<const char*>(b), count);
}

static auto equal(const void* const a, const void* const b, const size_t count) -> bool {
	return ca::equal_units(static_cast<const char*>(a), static_cast<const char*>(b), count);
}
#endif


// -- CASES ------------------------------------------------------------------

NI static auto fixed(const size_t size, const size_t from, const size_t to) -> void {
	const size_t count = opaque + size;
	const size_t rounds = TOTAL / (size < 64U ? 64U : size) / (size >= BUFFER / 4U ? 4U : 1U);
	size_t sum = 0;
	for (size_t i = 0; i < rounds; ++i) {
		const char* a = left + from;
		asm volatile("" : "+r"(a));
		sum += static_cast<size_t>(compare(a, right + to, count));
	}
	sink = sum;
}

NI static auto fixed_equal(const size_t size) -> void {
	const size_t count = opaque + size;
	const size_t rounds = TOTAL / (size < 64U ? 64U : size) / (size >= BUFFER / 4U ? 4U : 1U);
	size_t sum = 0;
	for (size_t i = 0; i < rounds; ++i) {
		const char* a = left;
		asm volatile("" : "+r"(a));
		sum += equal(a, right, count);
	}
	sink = sum;
}

/* A 256-byte range differing at a place the branch predictor cannot learn. */
NI static auto differing(void) -> void {
	const size_t rounds = TOTAL / 256U / 4U;
	size_t sum = opaque;
	for (size_t i = 0; i < rounds; ++i) {
		const size_t slot = (i * 2654435761U >> 7U) % 4096U;
		sum += static_cast<size_t>(compare(left + BUFFER / 2U + slot * 256U, right + BUFFER / 2U + slot * 256U, 256U));
	}
	sink = sum;
}

NI static auto mixed(const bool only_equal) -> void {
	const size_t rounds = 4096U * 16U;
	size_t sum = opaque;
	for (size_t round = 0; round < rounds; ++round) {
		for (int i = 0; i < MIX; i += 16) {
			const unsigned offset = mix_offsets[(static_cast<size_t>(i) + round) % MIX];
			const unsigned size = mix_sizes[i + static_cast<int>(round % 16U)];
			sum += only_equal ? equal(left + offset, right + offset, size) : static_cast<size_t>(compare(left + offset, right + offset, size));
		}
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
	for (size_t i = 0; i < sizeof left; ++i) {
		left[i] = static_cast<char>('a' + i % 26U);
		right[i] = left[i];
	}
	for (size_t slot = 0; slot < 4096U; ++slot) {
		const size_t at = BUFFER / 2U + slot * 256U + (slot * 40503U >> 3U) % 256U;
		right[at] = static_cast<char>(right[at] + 1);
	}
	for (unsigned i = 0; i < MIX; ++i) {
		mix_sizes[i] = mix_size(i);
		mix_offsets[i] = (i * 2246822519U >> 7U) % 2048U;
	}
	if (!open_counters())
		return 1;
	static const size_t sizes[] = {1, 7, 16, 33, 64, 100, 256, 300, 1024, 4096, 65536, 1U << 20U};
	static const char* const names[] = {"1", "7", "16", "33", "64", "100", "256", "300", "1K", "4K", "64K", "1M"};
	char name[64];
	for (size_t i = 0; i < sizeof sizes / sizeof *sizes; ++i) {
		snprintf(name, sizeof name, "compare %s", names[i]);
		bench(name, [=] { fixed(sizes[i], 0U, 0U); });
	}
	for (size_t i = 0; i < sizeof sizes / sizeof *sizes; ++i) {
		snprintf(name, sizeof name, "equal %s", names[i]);
		bench(name, [=] { fixed_equal(sizes[i]); });
	}
	bench("compare 100 +1/+3", [] { fixed(100U, 1U, 3U); });
	bench("compare 4K +1/+3", [] { fixed(4096U, 1U, 3U); });
	bench("differ in 256", differing);
	bench("mix compare 0-2K", [] { mixed(false); });
	bench("mix equal 0-2K", [] { mixed(true); });
	return 0;
}
