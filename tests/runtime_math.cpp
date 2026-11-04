// expect: exit 0
// link: freestanding
// The math builtins compilers lower to libm calls link against libca_runtime alone and give
// exact results, errno set as glibc sets it. Operands pass through an asm so nothing is folded.
#include "3-system/error/last_error.hpp"

template <typename T>
static auto opaque(T value) -> T {
	asm("" : "+m"(value));
	return value;
}

static auto errno_after(const int expected) -> bool {
	const bool same = ca::sys::impl::last_code == expected;
	ca::sys::impl::last_code = 0;
	return same;
}

auto main(void) -> int {
	const double seven = opaque(7.0);
	const double two = opaque(2.0);
	const double third = opaque(0.1);
	if (__builtin_fmod(seven, two) != 1.0 || __builtin_fmod(opaque(1e300), third) != 0.00011215964963492975)
		return 1;
	if (__builtin_fmodf(opaque(7.5F), opaque(2.0F)) != 1.5F || __builtin_fmodl(opaque(-7.5L), opaque(2.0L)) != -1.5L)
		return 2;
	if (__builtin_fmod(seven, opaque(0.0)) == __builtin_fmod(seven, opaque(0.0)) || !errno_after(33))
		return 3;
	if (__builtin_remainder(seven, two) != -1.0 || __builtin_remainderf(opaque(5.0F), opaque(2.0F)) != 1.0F)
		return 4;
	int quotient = 0;
	if (__builtin_remquo(seven, two, &quotient) != -1.0 || quotient != 4)
		return 5;
	if (__builtin_sqrt(two) != 1.4142135623730951 || __builtin_sqrtl(opaque(4.0L)) != 2.0L)
		return 6;
	if (__builtin_sqrt(opaque(-1.0)) == __builtin_sqrt(opaque(-1.0)) || !errno_after(33))
		return 7;
	const double half = opaque(-2.5);
	if (__builtin_floor(half) != -3.0 || __builtin_ceil(half) != -2.0 || __builtin_trunc(half) != -2.0)
		return 8;
	if (__builtin_round(half) != -3.0 || __builtin_roundeven(half) != -2.0 || __builtin_rint(half) != -2.0)
		return 9;
	if (__builtin_nearbyint(half) != -2.0 || __builtin_lrint(half) != -2 || __builtin_llround(half) != -3)
		return 10;
	if (__builtin_floorf(opaque(1.5F)) != 1.0F || __builtin_ceill(opaque(1.25L)) != 2.0L || __builtin_lroundl(opaque(2.5L)) != 3)
		return 11;
	if (__builtin_fmax(opaque(-0.0), opaque(1.0)) != 1.0 || __builtin_fmin(opaque(__builtin_nan("")), two) != 2.0)
		return 12;
	if (__builtin_fminl(opaque(3.0L), opaque(-3.0L)) != -3.0L || __builtin_fdim(seven, two) != 5.0)
		return 13;
	if (__builtin_fma(opaque(0x1p-30), opaque(0x1p-30), opaque(1.0)) != 1.0000000000000000008673617379884035)
		return 14;
	if (__builtin_fmaf(opaque(1.0F + 0x1p-23F), opaque(1.0F - 0x1p-23F), opaque(-1.0F)) != -0x1p-46F)
		return 15;
	if (__builtin_ldexp(opaque(0.75), 4) != 12.0 || __builtin_scalbn(opaque(1.0), -1074) != 0x1p-1074)
		return 16;
	if (__builtin_ldexp(opaque(1.0), 2000) != __builtin_inf() || !errno_after(34))
		return 17;
	int exponent = 0;
	double whole = 0.0;
	if (__builtin_frexp(opaque(12.0), &exponent) != 0.75 || exponent != 4 || __builtin_modf(half, &whole) != -0.5 || whole != -2.0)
		return 18;
	if (__builtin_logb(opaque(0x1p-1074)) != -1074.0 || __builtin_ilogb(opaque(1024.0)) != 10 || __builtin_ilogbl(opaque(3.0L)) != 1)
		return 19;
	if (__builtin_nextafter(opaque(1.0), two) != 1.0000000000000002 || __builtin_nextafterf(opaque(0.0F), opaque(1.0F)) != 0x1p-149F)
		return 20;
	return 0;
}
