// expect: exit 0
#include "1-primitives/memory/impl/compare_bytes.hpp"
#include "1-primitives/memory/impl/cpu_features.hpp"
#include "1-primitives/memory/impl/fill_bytes.hpp"
#include "3-system/memory/mmap_anonymous.hpp"
#include "3-system/memory/mprotect.hpp"
#include "3-system/memory/munmap.hpp"
#include "3-system/memory/page_size.hpp"
#include "3-system/memory/protection.hpp"

extern "C" auto memset(void* to, int value, ca::usz count) noexcept -> void*;
extern "C" auto memcmp(const void* a, const void* b, ca::usz count) noexcept -> int;
extern "C" auto bcmp(const void* a, const void* b, ca::usz count) noexcept -> int;

static auto (*volatile memset_c)(void*, int, ca::usz) noexcept -> void* = &memset;
static auto (*volatile memcmp_c)(const void*, const void*, ca::usz) noexcept -> int = &memcmp;
static auto (*volatile bcmp_c)(const void*, const void*, ca::usz) noexcept -> int = &bcmp;

/* Every range ends at a page followed by a PROT_NONE one, or starts right after one: a load or
   store outside [p, p + count) faults. */
static auto run(char* const low, char* const high, const ca::usz page, const bool avx2) -> int {
	for (ca::usz count = 0U; count <= 2U * page; count += count < 600U ? 1U : 97U) {
		char* const ends[] = {low + 2U * page - count, high, low + 2U * page - count};
		char* const others[] = {high, low + 2U * page - count, low + 2U * page - count};
		for (int k = 0; k < 3; ++k) {
			char* const a = ends[k];
			char* const b = others[k];
			if (memset_c(a, 'x', count) != a || memset_c(b, 'x', count) != b)
				return 1;
			if (ca::impl::fill_bytes_sse2(a, 'x', count) != a)
				return 2;
			if (avx2 && ca::impl::fill_bytes_avx2(a, 'x', count) != a)
				return 3;
			if (memcmp_c(a, b, count) != 0 || bcmp_c(a, b, count) != 0)
				return 4;
			if (ca::impl::compare_bytes_sse2(a, b, count) != 0 || ca::impl::differ_bytes_sse2(a, b, count) != 0)
				return 5;
			if (avx2 && (ca::impl::compare_bytes_avx2(a, b, count) != 0 || ca::impl::differ_bytes_avx2(a, b, count) != 0))
				return 6;
		}
	}
	return 0;
}

auto main(void) -> int {
	const ca::usz page = ca::sys::page_size();
	auto map = ca::sys::mmap_anonymous(7U * page, ca::sys::protection::read | ca::sys::protection::write);
	if (!map)
		return 10;
	const ca::span<ca::byte> bytes = *map;
	char* const base = reinterpret_cast<char*>(bytes.data());
	if (!ca::sys::mprotect(bytes.sub(0U, page), ca::sys::protection::none) || !ca::sys::mprotect(bytes.sub(3U * page, page), ca::sys::protection::none) || !ca::sys::mprotect(bytes.sub(6U * page, page), ca::sys::protection::none))
		return 11;
	const ca::impl::cpu_features cpu = ca::impl::read_cpu_features();
	const int status = run(base + page, base + 4U * page, page, cpu._avx2 && cpu._bmi2);
	return status != 0 ? status : ca::sys::munmap(bytes) ? 0 : 12;
}
