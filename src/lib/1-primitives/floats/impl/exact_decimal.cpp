#include "1-primitives/config/compiler.hpp"
#include "1-primitives/floats/impl/exact_decimal.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"

/* Layer 1 calls nothing outside ca: no loop turned into memcpy or memset, no stack protector
   (its failure handler is libc's or libca_runtime's). */
#if ca_compiler(clang)
#	pragma clang attribute push(__attribute__((no_builtin, no_stack_protector)), apply_to = function)
#else
#	pragma GCC optimize("no-tree-loop-distribute-patterns", "no-stack-protector")
#endif


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- EXACT DECIMAL ------------------------------------------------------

	namespace impl {

		/* @DOC An unsigned integer of up to LIMBS 32-bit limbs, least significant first: enough
		   for 2^53 * 5^1074 (2547 bits). */
		struct big_integer final {
				using self = ca::impl::big_integer;
				static constexpr ca::usz LIMBS = 84U;
				ca::u32 _limbs[LIMBS];
				ca::usz _size;
		}; // struct big_integer

	} // namespace impl

} // namespace ca

static auto multiply_small(ca::impl::big_integer& n, const ca::u32 factor) noexcept -> void {

	ca::u64 carry = 0U;
	for (ca::usz i = 0U; i < n._size; ++i) {
		const ca::u64 product = static_cast<ca::u64>(n._limbs[i]) * factor + carry;
		n._limbs[i] = static_cast<ca::u32>(product);
		      carry = product >> 32U;
	}
	if (carry != 0U)
		n._limbs[n._size++] = static_cast<ca::u32>(carry);
}

static auto shift_left(ca::impl::big_integer& n, const ca::usz bits) noexcept -> void {

	const ca::usz limbs = bits / 32U;
	const ca::usz rest = bits % 32U;
	if (rest != 0U) {
		ca::u32 carry = 0U;
		for (ca::usz i = 0U; i < n._size; ++i) {
			const ca::u32 limb = n._limbs[i];
			n._limbs[i] = limb << rest | carry;
			      carry = limb >> (32U - rest);
		}
		if (carry != 0U)
			n._limbs[n._size++] = carry;
	}
	if (limbs != 0U) {
		for (ca::usz i = n._size; i-- > 0U;)
			n._limbs[i + limbs] = n._limbs[i];
		for (ca::usz i = 0U; i < limbs; ++i)
			n._limbs[i] = 0U;
		n._size += limbs;
	}
}

/* n / 10^9 in place; the remainder. */
static auto divide_billion(ca::impl::big_integer& n) noexcept -> ca::u32 {

	ca::u64 remainder = 0U;
	for (ca::usz i = n._size; i-- > 0U;) {
		const ca::u64 current = remainder << 32U | n._limbs[i];
		n._limbs[i] = static_cast<ca::u32>(current / 1000000000U);
		  remainder = current % 1000000000U;
	}
	while (n._size != 0U && n._limbs[n._size - 1U] == 0U)
		--n._size;
	return static_cast<ca::u32>(remainder);
}

auto ca::impl::exact_decimal_of(const ca::f64 value, ca::impl::exact_decimal& out) noexcept -> void {

	const ca::u64 bits = __builtin_bit_cast(ca::u64, value);
	const ca::u32 field = static_cast<ca::u32>(bits >> 52U) & 0x7ffU;
	ca::u64 mantissa = bits & ((ca::u64{1} << 52U) - 1U);
	ca::i32 exponent = -1074;
	if (field != 0U) {
		mantissa |= ca::u64{1} << 52U;
		exponent = static_cast<ca::i32>(field) - 1075;
	}
	while ((mantissa & 1U) == 0U && exponent < 0) {
		mantissa >>= 1U;
		++exponent;
	}
	ca::impl::big_integer n;
	n._limbs[0] = static_cast<ca::u32>(mantissa);
	n._limbs[1] = static_cast<ca::u32>(mantissa >> 32U);
	    n._size = n._limbs[1] != 0U ? 2U : 1U;
	ca::i32 scale = 0;
	if (exponent >= 0) {
		shift_left(n, static_cast<ca::usz>(exponent));
	} else {
		scale = -exponent;
		ca::i32 left = scale;
		for (; left >= 13; left -= 13)
			multiply_small(n, 1220703125U);
		ca::u32 rest = 1U;
		for (; left > 0; --left)
			rest *= 5U;
		multiply_small(n, rest);
	}
	ca::u32 chunks[ca::impl::exact_decimal::CAPACITY / 9U + 1U];
	ca::usz count = 0U;
	while (n._size != 0U)
		chunks[count++] = divide_billion(n);
	ca::usz length = 0U;
	for (ca::usz i = count; i-- > 0U;) {
		const ca::u32 chunk = chunks[i];
		char text[9];
		ca::u32 rest = chunk;
		for (ca::usz k = 9U; k-- > 0U;) {
			text[k] = static_cast<char>('0' + rest % 10U);
			rest /= 10U;
		}
		ca::usz from = 0U;
		if (length == 0U) {
			while (from < 8U && text[from] == '0')
				++from;
		}
		for (; from < 9U; ++from)
			out._digits[length++] = text[from];
	}
	out._count = length;
	out._point = static_cast<ca::i32>(length) - scale;
}

#if ca_compiler(clang)
#	pragma clang attribute pop
#endif
