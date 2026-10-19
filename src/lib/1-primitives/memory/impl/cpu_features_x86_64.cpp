#include "1-primitives/memory/impl/cpu_features.hpp"

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"

#if ca_architecture(x86_64)


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- CPU FEATURES X86 64 ------------------------------------------------

	namespace impl {

		/* @DOC eax, ebx, ecx and edx after cpuid. */
		struct cpuid_registers final {
				using self = ca::impl::cpuid_registers;
				ca::u32 _a;
				ca::u32 _b;
				ca::u32 _c;
				ca::u32 _d;
		}; // struct cpuid_registers

	} // namespace impl

} // namespace ca


// -- DETECTION --------------------------------------------------------------

static auto cpuid(const ca::u32 leaf, const ca::u32 subleaf) noexcept -> ca::impl::cpuid_registers {

	ca::impl::cpuid_registers r;
	asm("cpuid" : "=a"(r._a), "=b"(r._b), "=c"(r._c), "=d"(r._d) : "a"(leaf), "c"(subleaf));
	return r;
}

static auto xgetbv(void) noexcept -> ca::u64 {

	ca::u32 low;
	ca::u32 high;
	asm("xgetbv" : "=a"(low), "=d"(high) : "c"(0U));
	return static_cast<ca::u64>(high) << 32U | low;
}

static auto last_level_cache(const ca::u32 leaf) noexcept -> ca::usz {

	ca::usz size = 0U;
	for (ca::u32 index = 0U; index < 16U; ++index) {
		const ca::impl::cpuid_registers r = cpuid(leaf, index);
		if ((r._a & 31U) == 0U)
			break;
		const ca::usz ways = (r._b >> 22U) + 1U;
		const ca::usz partitions = (r._b >> 12U & 1023U) + 1U;
		const ca::usz line = (r._b & 4095U) + 1U;
		const ca::usz sets = static_cast<ca::usz>(r._c) + 1U;
		size = ways * partitions * line * sets;
	}
	return size;
}

auto ca::impl::read_cpu_features(void) noexcept -> ca::impl::cpu_features {

	const ca::u32 top = cpuid(0U, 0U)._a;
	const ca::u32 extended = cpuid(0x80000000U, 0U)._a;
	const ca::impl::cpuid_registers basic = cpuid(1U, 0U);
	const ca::impl::cpuid_registers more = top >= 7U ? cpuid(7U, 0U) : ca::impl::cpuid_registers{};
	const bool xsave = (basic._c >> 27U & 1U) != 0U;
	const bool avx = xsave && (basic._c >> 28U & 1U) != 0U && (xgetbv() & 6U) == 6U;
	ca::usz cache = top >= 4U ? last_level_cache(4U) : 0U;
	if (cache == 0U && extended >= 0x8000001DU)
		cache = last_level_cache(0x8000001DU);
	if (cache == 0U)
		cache = static_cast<ca::usz>(8U) << 20U;
	return {avx && (more._b >> 5U & 1U) != 0U, (more._b >> 8U & 1U) != 0U, (more._b >> 9U & 1U) != 0U, cache};
}

#endif
