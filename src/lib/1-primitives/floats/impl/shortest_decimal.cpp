/* Ryu (Ulf Adams, "Ryu: fast float-to-string conversion", PLDI 2018), the double algorithm
   (d2s) run for both f64 and f32: it only needs the binary mantissa m2, exponent e2 and the
   rounding interval, and the f32's interval is narrower, so 0.1F comes out as 1e-1. The value
   is mv = 4 m2 at 2^(e2), its neighbours' midpoints vp = mv + 2 and vm = mv - 1 - (1 at a
   power of two), all three scaled to 10^e10 by one 128-bit product each; digits are then
   removed while vp and vm still differ, rounding the last one to nearest, ties to even. */

#include "1-primitives/floats/impl/power5_tables.hpp"
#include "1-primitives/floats/impl/shortest_decimal.hpp"
#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SHORTEST DECIMAL ---------------------------------------------------

	namespace impl {

		__extension__ typedef unsigned __int128 ryu_wide;

	} // namespace impl

} // namespace ca

static auto pow5_bits(const ca::i32 e) noexcept -> ca::i32 {
	return static_cast<ca::i32>((static_cast<ca::u32>(e) * 1217359U) >> 19U) + 1;
}

static auto log10_pow2(const ca::i32 e) noexcept -> ca::i32 {
	return static_cast<ca::i32>((static_cast<ca::u32>(e) * 78913U) >> 18U);
}

static auto log10_pow5(const ca::i32 e) noexcept -> ca::i32 {
	return static_cast<ca::i32>((static_cast<ca::u32>(e) * 732923U) >> 20U);
}

static auto pow5_factor(ca::u64 value) noexcept -> ca::i32 {

	ca::i32 count = 0;
	for (;;) {
		const ca::u64 quotient = value / 5U;
		if (value - 5U * quotient != 0U)
			return count;
		value = quotient;
		++count;
	}
}

static auto multiple_of_power5(const ca::u64 value, const ca::i32 p) noexcept -> bool {
	return pow5_factor(value) >= p;
}

static auto multiple_of_power2(const ca::u64 value, const ca::i32 p) noexcept -> bool {
	return (value & ((ca::u64{1} << p) - 1U)) == 0U;
}

/* (m * multiplier) >> shift, multiplier the 128 bits {low, high}, shift >= 64. */
static auto multiply_shift(const ca::u64 m, const ca::u64* const multiplier, const ca::i32 shift) noexcept -> ca::u64 {

	const ca::impl::ryu_wide low = static_cast<ca::impl::ryu_wide>(m) * multiplier[0];
	const ca::impl::ryu_wide high = static_cast<ca::impl::ryu_wide>(m) * multiplier[1];
	return static_cast<ca::u64>(((low >> 64U) + high) >> (shift - 64));
}

static auto shortest(const ca::u64 m2, const ca::i32 e2, const ca::u64 mm_shift) noexcept -> ca::impl::decimal_float {

	const bool accept_bounds = (m2 & 1U) == 0U;
	const ca::u64 mv = 4U * m2;
	ca::u64 vr;
	ca::u64 vp;
	ca::u64 vm;
	ca::i32 e10;
	bool vm_is_trailing_zeros = false;
	bool vr_is_trailing_zeros = false;
	if (e2 >= 0) {
		const ca::i32 q = log10_pow2(e2) - (e2 > 3 ? 1 : 0);
		e10 = q;
		const ca::i32 k = static_cast<ca::i32>(ca::impl::POWER5_BITS) + pow5_bits(q) - 1;
		const ca::i32 i = -e2 + q + k;
		vr = multiply_shift(mv, ca::impl::POWER5_INVERSE[q], i);
		vp = multiply_shift(mv + 2U, ca::impl::POWER5_INVERSE[q], i);
		vm = multiply_shift(mv - 1U - mm_shift, ca::impl::POWER5_INVERSE[q], i);
		if (q <= 21) {
			if (mv % 5U == 0U)
				vr_is_trailing_zeros = multiple_of_power5(mv, q);
			else if (accept_bounds)
				vm_is_trailing_zeros = multiple_of_power5(mv - 1U - mm_shift, q);
			else
				vp -= multiple_of_power5(mv + 2U, q) ? 1U : 0U;
		}
	} else {
		const ca::i32 q = log10_pow5(-e2) - (-e2 > 1 ? 1 : 0);
		e10 = q + e2;
		const ca::i32 i = -e2 - q;
		const ca::i32 k = pow5_bits(i) - static_cast<ca::i32>(ca::impl::POWER5_BITS);
		const ca::i32 j = q - k;
		vr = multiply_shift(mv, ca::impl::POWER5[i], j);
		vp = multiply_shift(mv + 2U, ca::impl::POWER5[i], j);
		vm = multiply_shift(mv - 1U - mm_shift, ca::impl::POWER5[i], j);
		if (q <= 1) {
			vr_is_trailing_zeros = true;
			if (accept_bounds)
				vm_is_trailing_zeros = mm_shift == 1U;
			else
				--vp;
		} else if (q < 63) {
			vr_is_trailing_zeros = multiple_of_power2(mv, q);
		}
	}
	ca::i32 removed = 0;
	ca::u64 last_removed_digit = 0U;
	ca::u64 output;
	if (vm_is_trailing_zeros || vr_is_trailing_zeros) {
		while (vp / 10U > vm / 10U) {
			vm_is_trailing_zeros = vm_is_trailing_zeros && vm % 10U == 0U;
			vr_is_trailing_zeros = vr_is_trailing_zeros && last_removed_digit == 0U;
			  last_removed_digit = vr % 10U;
			vr /= 10U;
			vp /= 10U;
			vm /= 10U;
			++removed;
		}
		if (vm_is_trailing_zeros) {
			while (vm % 10U == 0U) {
				vr_is_trailing_zeros = vr_is_trailing_zeros && last_removed_digit == 0U;
				  last_removed_digit = vr % 10U;
				vr /= 10U;
				vp /= 10U;
				vm /= 10U;
				++removed;
			}
		}
		if (vr_is_trailing_zeros && last_removed_digit == 5U && vr % 2U == 0U)
			last_removed_digit = 4U;
		const bool up = (vr == vm && (!accept_bounds || !vm_is_trailing_zeros)) || last_removed_digit >= 5U;
		output = vr + (up ? 1U : 0U);
	} else {
		bool round_up = false;
		if (vp / 100U > vm / 100U) {
			round_up = vr % 100U >= 50U;
			vr /= 100U;
			vp /= 100U;
			vm /= 100U;
			removed += 2;
		}
		while (vp / 10U > vm / 10U) {
			round_up = vr % 10U >= 5U;
			vr /= 10U;
			vp /= 10U;
			vm /= 10U;
			++removed;
		}
		output = vr + ((vr == vm || round_up) ? 1U : 0U);
	}
	return ca::impl::decimal_float{output, e10 + removed};
}

[[nodiscard]] auto ca::impl::shortest_decimal(const ca::f64 value) noexcept -> ca::impl::decimal_float {

	const ca::u64 bits = __builtin_bit_cast(ca::u64, value);
	const ca::u64 mantissa = bits & ((ca::u64{1} << 52U) - 1U);
	const ca::u32 exponent = static_cast<ca::u32>(bits >> 52U) & 0x7ffU;
	const ca::u64 mm_shift = mantissa != 0U || exponent <= 1U ? 1U : 0U;
	if (exponent == 0U)
		return shortest(mantissa, 1 - 1023 - 52 - 2, mm_shift);
	return shortest((ca::u64{1} << 52U) | mantissa, static_cast<ca::i32>(exponent) - 1023 - 52 - 2, mm_shift);
}

[[nodiscard]] auto ca::impl::shortest_decimal(const ca::f32 value) noexcept -> ca::impl::decimal_float {

	const ca::u32 bits = __builtin_bit_cast(ca::u32, value);
	const ca::u64 mantissa = bits & ((ca::u32{1} << 23U) - 1U);
	const ca::u32 exponent = (bits >> 23U) & 0xffU;
	const ca::u64 mm_shift = mantissa != 0U || exponent <= 1U ? 1U : 0U;
	if (exponent == 0U)
		return shortest(mantissa, 1 - 127 - 23 - 2, mm_shift);
	return shortest((ca::u64{1} << 23U) | mantissa, static_cast<ca::i32>(exponent) - 127 - 23 - 2, mm_shift);
}
