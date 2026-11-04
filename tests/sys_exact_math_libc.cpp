// expect: exit 0
// link: libc
// ca's exact math against glibc's libm, bit for bit (NaNs included) and errno, for float, double
// and long double: special values (zeros, subnormals, halves, 2^p, the largest, infinities,
// quiet and signaling NaNs) and pseudo-random ones, in each of the 4 rounding modes. libm is
// dlopen'd: neither compiler folds its calls, and clang links no libm. fma also without the
// instruction.
#include "1-primitives/bit/bit_cast.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_floating_point.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/math/impl/exact_math.hpp"

extern "C" auto dlopen(const char* file, int mode) noexcept -> void*;
extern "C" auto dlsym(void* handle, const char* name) noexcept -> void*;
extern "C" auto __errno_location(void) noexcept -> int*;
extern "C" auto printf(const char* format, ...) noexcept -> int;

struct extended final {
	ca::u64 _significand;
	ca::u16 _sign_exponent;
	ca::u16 _padding[3];
};

static void* libm = nullptr;
static int failures = 0;
static ca::u64 state = 0x9E3779B97F4A7C15ULL;

static auto next(void) -> ca::u64 {
	state ^= state << 13U;
	state ^= state >> 7U;
	state ^= state << 17U;
	return state;
}

template <typename F>
static auto glibc(const char* name) -> F* {
	return reinterpret_cast<F*>(dlsym(libm, name));
}

static auto set_rounding(const unsigned mode) -> void {
	unsigned control;
	asm volatile("stmxcsr %0" : "=m"(control));
	control = (control & ~0x6000U) | mode << 13U;
	asm volatile("ldmxcsr %0" : : "m"(control));
	ca::u16 word;
	asm volatile("fnstcw %0" : "=m"(word));
	word = static_cast<ca::u16>((word & ~0x0C00U) | mode << 10U);
	asm volatile("fldcw %0" : : "m"(word));
}


// -- VALUES -----------------------------------------------------------------

static auto bits_of(const float x) -> ca::u64 {
	return ca::bit_cast<ca::u32>(x);
}

static auto bits_of(const double x) -> ca::u64 {
	return ca::bit_cast<ca::u64>(x);
}

static auto bits_of(const long double x) -> ca::u64 {
	const extended e = ca::bit_cast<extended>(x);
	return static_cast<ca::u64>(e._sign_exponent) << 48U | e._significand >> 16U;
}

template <typename T>
static auto same(const T a, const T b) -> bool {
	if constexpr (sizeof(T) == 16U) {
		const extended ea = ca::bit_cast<extended>(a);
		const extended eb = ca::bit_cast<extended>(b);
		return ea._significand == eb._significand && ea._sign_exponent == eb._sign_exponent;
	} else if constexpr (sizeof(T) == 4U || sizeof(T) == 8U) {
		if constexpr (ca::is_floating_point<T>)
			return bits_of(a) == bits_of(b);
		else
			return a == b;
	} else {
		return a == b;
	}
}

static auto from(const ca::u64 b, float) -> float {
	return ca::bit_cast<float>(static_cast<ca::u32>(b));
}

static auto from(const ca::u64 b, double) -> double {
	return ca::bit_cast<double>(b);
}

static auto from(const ca::u64 b, long double) -> long double {
	extended e{};
	ca::u16 field = static_cast<ca::u16>(b >> 48U & 0x7FFFU);
	e._significand = b << 16U | (next() & 0xFFFFU);
	if (field != 0U)
		e._significand |= 1ULL << 63U;
	else
		e._significand &= ~(1ULL << 63U);
	e._sign_exponent = static_cast<ca::u16>(field | (b >> 63U) << 15U);
	return ca::bit_cast<long double>(e);
}

template <typename T>
static auto infinity(void) -> T {
	if constexpr (sizeof(T) == 4U)
		return __builtin_inff();
	else if constexpr (sizeof(T) == 8U)
		return __builtin_inf();
	else
		return __builtin_infl();
}

/* A NaN of T itself, its payload in the low bits: never converted, which quiets. */
template <typename T>
static auto nan_with(const bool signaling, const ca::u64 payload) -> T {
	if constexpr (sizeof(T) == 4U)
		return from((signaling ? 0x7F800000U : 0x7FC00000U) | (payload != 0U ? payload : 1U * signaling), T{});
	else if constexpr (sizeof(T) == 8U)
		return from((signaling ? 0x7FF0000000000000ULL : 0x7FF8000000000000ULL) | (payload != 0U ? payload : 1U * signaling), T{});
	else {
		extended e{};
		e._significand = (signaling ? 0x8000000000000000ULL : 0xC000000000000000ULL) | (payload != 0U ? payload : 1U * signaling);
		e._sign_exponent = 0x7FFFU;
		return ca::bit_cast<long double>(e);
	}
}

template <typename T>
static auto quiet(const ca::u64 payload = 0U) -> T {
	return nan_with<T>(false, payload);
}

template <typename T>
static auto signaling(const ca::u64 payload = 0U) -> T {
	return nan_with<T>(true, payload);
}

constexpr ca::usz COUNT = 3000U;

/* Special values, then values of random bits, then random values of moderate exponent. */
template <typename T>
static auto fill(T* const out) -> ca::usz {
	constexpr int digits = sizeof(T) == 4U ? 24 : sizeof(T) == 8U ? 53 : 64;
	const T big = static_cast<T>(1ULL << (digits > 63 ? 63 : digits));
	const T specials[] = {
		T{0}, T{0.5}, T{1}, T{1.5}, T{2}, T{2.5}, T{3}, T{0.25}, T{0.75}, T{7}, T{0.1}, T{10},
		static_cast<T>(0.49999999999999994), T{0.49999997F}, big, big - T{0.5}, big + T{1}, big * T{2}, big * big,
		__FLT_MIN__, __FLT_MAX__, __FLT_DENORM_MIN__, __FLT_EPSILON__,
		static_cast<T>(__DBL_MIN__), static_cast<T>(__DBL_MAX__), static_cast<T>(__DBL_DENORM_MIN__),
		static_cast<T>(__LDBL_MIN__), static_cast<T>(__LDBL_MAX__), static_cast<T>(__LDBL_DENORM_MIN__),
		static_cast<T>(__LDBL_MIN__ / 3.0L), static_cast<T>(__DBL_MIN__ / 3.0), __FLT_MIN__ / T{3},
		static_cast<T>(9223372036854775807.0L), static_cast<T>(0x1p63L - 0.5L), static_cast<T>(0x1p62L + 0.5L),
		static_cast<T>(1e300), static_cast<T>(1e-300), static_cast<T>(1e30F), T{1e-30F},
		infinity<T>(), quiet<T>(), signaling<T>(), quiet<T>(0x1234U), signaling<T>(0x5678U),
	};
	ca::usz n = 0U;
	for (const T s : specials) {
		out[n++] = s;
		out[n++] = -s;
	}
	while (n < COUNT / 2U)
		out[n++] = from(next(), T{});
	while (n < COUNT) {
		const ca::u64 r = next();
		const ca::u64 b = sizeof(T) == 4U ? (r & 0x807FFFFFU) | (static_cast<ca::u64>(110U + r % 40U) << 23U)
		                : sizeof(T) == 8U ? (r & 0x800FFFFFFFFFFFFFULL) | ((1000ULL + (r >> 20U) % 60U) << 52U)
		                                  : (r & 0x8000FFFFFFFFFFFFULL) | ((16360ULL + (r >> 20U) % 60U) << 48U);
		out[n++] = from(b, T{});
	}
	return n;
}

template <typename T>
static auto pick(const T* const values, const ca::usz n) -> T {
	return values[next() % n];
}


// -- CHECKS -----------------------------------------------------------------

static auto reset(void) -> void {
	*__errno_location() = 0;
	ca::sys::impl::last_code = 0;
}

static char last_name[32];
static int shown = 0;

/* Counts a failure; true for the first 3 of each function, which are printed. */
static auto show(const char* const name) -> bool {
	++failures;
	ca::usz at = 0U;
	while (name[at] != 0 && name[at] == last_name[at])
		++at;
	if (name[at] != last_name[at]) {
		for (at = 0U; name[at] != 0; ++at)
			last_name[at] = name[at];
		last_name[at] = 0;
		shown = 0;
	}
	return shown++ < 3;
}

static auto errno_same(const char* name, const unsigned mode, const ca::u64 x, const ca::u64 y) -> bool {
	if (*__errno_location() == ca::sys::impl::last_code)
		return true;
	if (show(name))
		printf("%s mode %u (%016llx, %016llx): errno %d, ca %d\n", name, mode, static_cast<unsigned long long>(x),
		       static_cast<unsigned long long>(y), *__errno_location(), ca::sys::impl::last_code);
	return false;
}

template <typename R>
static auto bits_or_value(const R r) -> ca::u64 {
	if constexpr (ca::is_floating_point<R>)
		return bits_of(r);
	else
		return static_cast<ca::u64>(r);
}

template <typename R>
static auto report(const char* name, const unsigned mode, const ca::u64 x, const ca::u64 y, const R want, const R got) -> void {
	if (show(name))
		printf("%s mode %u (%016llx, %016llx): glibc %016llx, ca %016llx\n", name, mode, static_cast<unsigned long long>(x),
		       static_cast<unsigned long long>(y), static_cast<unsigned long long>(bits_or_value(want)),
		       static_cast<unsigned long long>(bits_or_value(got)));
}

/* glibc's remainder of float and double, rounding down or up, or of double for y below 2^-1020,
   can give a zero remainder the sign opposite x's (x - y rounded): IEEE 754 gives it x's in
   every mode, as fprem1 does. */
static bool zero_sign_free = false;

template <typename T>
static auto zero_sign_differs(const T want, const T got, const unsigned mode, const T y) -> bool {
	const bool tiny = __builtin_fabsl(static_cast<long double>(y)) < 0x1p-1020L;
	return zero_sign_free && (mode != 0U || tiny) && want == T{0} && got == T{0};
}

/* Compares a result and an integer out-parameter, then errno. */
template <typename T, typename P>
static auto compare(const char* name, const unsigned mode, const T x, const T y, const T want, const T got,
                    const P want_part, const P got_part, const int want_errno) -> void {
	*__errno_location() = want_errno;
	if (!same(want, got) && !zero_sign_differs(want, got, mode, y))
		report(name, mode, bits_of(x), bits_of(y), want, got);
	else if (!same(want_part, got_part))
		report(name, mode, bits_of(x), bits_of(y), bits_or_value(want_part), bits_or_value(got_part));
	else
		errno_same(name, mode, bits_of(x), bits_of(y));
}

template <typename T>
static auto check_parts(const char* name, T (*const remquo)(T, T, int*), T (*const frexp)(T, int*), T (*const modf)(T, T*),
                        const T* const values, const ca::usz n, const unsigned mode) -> void {
	for (ca::usz i = 0U; i < 30000U; ++i) {
		const T x = i < 8100U ? values[i / 90U] : pick(values, n);
		const T y = i < 8100U ? values[i % 90U] : pick(values, n);
		int want_q = 77;
		int got_q = 77;
		reset();
		const T want = remquo(x, y, &want_q);
		const int want_errno = *__errno_location();
		const T got = ca::sys::impl::remquo(x, y, &got_q);
		zero_sign_free = true;
		compare(name, mode, x, y, want, got, want_q, got_q, want_errno);
		zero_sign_free = false;
	}
	for (ca::usz i = 0U; i < n; ++i) {
		int want_e = 77;
		int got_e = 77;
		reset();
		const T want = frexp(values[i], &want_e);
		int want_errno = *__errno_location();
		const T got = ca::sys::impl::frexp(values[i], &got_e);
		compare("frexp", mode, values[i], T{}, want, got, want_e, got_e, want_errno);
		T want_w = 7;
		T got_w = 7;
		reset();
		const T want_f = modf(values[i], &want_w);
		want_errno = *__errno_location();
		const T got_f = ca::sys::impl::modf(values[i], &got_w);
		compare("modf", mode, values[i], T{}, want_f, got_f, want_w, got_w, want_errno);
	}
}

template <typename T>
static auto check_scaling(const char* name, T (*const ldexp)(T, int), T (*const scalbln)(T, long),
                          const T* const values, const ca::usz n, const unsigned mode) -> void {
	const long exponents[] = {0, 1, -1, 23, -23, 52, -52, 64, -64, 126, -126, 127, -127, 149, -149, 150, -150, 1000, -1000,
	                          1022, -1022, 1023, -1023, 1074, -1074, 1075, -1075, 1100, -1100, 2000, -2000, 16382, -16382,
	                          16383, -16383, 16445, -16445, 16446, -16446, 20000, -20000, 2147483647, -2147483647 - 1,
	                          9223372036854775807L, -9223372036854775807L - 1};
	for (ca::usz i = 0U; i < n; ++i) {
		for (ca::usz k = 0U; k < sizeof exponents / sizeof exponents[0] + 8U; ++k) {
			const long e = k < sizeof exponents / sizeof exponents[0] ? exponents[k] : static_cast<long>(next() % 4400U) - 2200;
			const T x = values[i];
			if (e >= -2147483647L - 1 && e <= 2147483647L) {
				reset();
				const T want = ldexp(x, static_cast<int>(e));
				const int want_errno = *__errno_location();
				const T got = ca::sys::impl::scalbln(x, static_cast<int>(e));
				compare(name, mode, x, T{}, want, got, 0, 0, want_errno);
			}
			reset();
			const T want = scalbln(x, e);
			const int want_errno = *__errno_location();
			const T got = ca::sys::impl::scalbln(x, e);
			compare("scalbln", mode, x, static_cast<T>(e), want, got, 0, 0, want_errno);
		}
	}
}

template <typename T>
static auto check_fma(const char* name, T (*const fma)(T, T, T), const T* const values, const ca::usz n, const unsigned mode) -> void {
	const auto one = [&](const T x, const T y, const T z) {
		reset();
		const T want = fma(x, y, z);
		const int want_errno = *__errno_location();
		const T got = ca::sys::impl::fma(x, y, z);
		const T software = ca::sys::impl::fma_software(x, y, z);
		compare(name, mode, x, y, want, got, 0, 0, want_errno);
		compare("fma_software", mode, x, z, want, software, 0, 0, want_errno);
	};
	for (ca::usz i = 0U; i < 40U; ++i)
		for (ca::usz j = 0U; j < 40U; ++j)
			for (ca::usz k = 0U; k < 40U; ++k)
				one(values[i * 2U], values[j * 2U + 1U], values[k + (i & 1U)]);
	for (ca::usz i = 0U; i < 100000U; ++i) {
		const T x = pick(values, n);
		const T y = pick(values, n);
		const T product = x * y;
		const ca::u64 r = next() % 4U;
		const T z = r == 0U ? -product : r == 1U ? -product * static_cast<T>(1.0000001) : r == 2U ? product : pick(values, n);
		one(x, y, z);
	}
}

template <typename T, typename R, typename F>
static auto unary(const char* name, R (*const reference)(T), const F ours, const T* const values, const ca::usz n, const unsigned mode) -> void {
	for (ca::usz i = 0U; i < n; ++i) {
		reset();
		const R want = reference(values[i]);
		const int want_errno = *__errno_location();
		const R got = ours(values[i]);
		*__errno_location() = want_errno;
		if (!same(want, got))
			report(name, mode, bits_of(values[i]), 0U, want, got);
		else
			errno_same(name, mode, bits_of(values[i]), 0U);
	}
}

template <typename T, typename F>
static auto binary(const char* name, T (*const reference)(T, T), const F ours, const T* const values, const ca::usz n, const unsigned mode) -> void {
	const auto one = [&](const T x, const T y) {
		reset();
		const T want = reference(x, y);
		const int want_errno = *__errno_location();
		const T got = ours(x, y);
		*__errno_location() = want_errno;
		if (!same(want, got) && !zero_sign_differs(want, got, mode, y))
			report(name, mode, bits_of(x), bits_of(y), want, got);
		else
			errno_same(name, mode, bits_of(x), bits_of(y));
	};
	for (ca::usz i = 0U; i < 90U; ++i)
		for (ca::usz j = 0U; j < 90U; ++j)
			one(values[i], values[j]);
	for (ca::usz i = 0U; i < 30000U; ++i) {
		const T x = pick(values, n);
		one(x, pick(values, n));
	}
}

template <typename T>
static auto check_type(const char* const suffix) -> void {
	static T values[COUNT];
	const ca::usz n = fill(values);
	static char names[4][32];
	static ca::usz turn = 0U;
	const auto named = [&](const char* base) {
		char* const name = names[turn++ % 4U];
		ca::usz at = 0U;
		for (; base[at] != 0; ++at)
			name[at] = base[at];
		for (ca::usz k = 0U; suffix[k] != 0; ++k)
			name[at++] = suffix[k];
		name[at] = 0;
		return static_cast<const char*>(name);
	};
	for (unsigned mode = 0U; mode < 4U; ++mode) {
		set_rounding(mode);
		unary(named("sqrt"), glibc<T(T)>(named("sqrt")), [](T x) { return ca::sys::impl::sqrt(x); }, values, n, mode);
		unary(named("floor"), glibc<T(T)>(named("floor")), [](T x) { return ca::sys::impl::floor(x); }, values, n, mode);
		unary(named("ceil"), glibc<T(T)>(named("ceil")), [](T x) { return ca::sys::impl::ceil(x); }, values, n, mode);
		unary(named("trunc"), glibc<T(T)>(named("trunc")), [](T x) { return ca::sys::impl::trunc(x); }, values, n, mode);
		unary(named("round"), glibc<T(T)>(named("round")), [](T x) { return ca::sys::impl::round(x); }, values, n, mode);
		unary(named("roundeven"), glibc<T(T)>(named("roundeven")), [](T x) { return ca::sys::impl::roundeven(x); }, values, n, mode);
		unary(named("rint"), glibc<T(T)>(named("rint")), [](T x) { return ca::sys::impl::rint(x); }, values, n, mode);
		unary(named("nearbyint"), glibc<T(T)>(named("nearbyint")), [](T x) { return ca::sys::impl::nearbyint(x); }, values, n, mode);
		unary(named("logb"), glibc<T(T)>(named("logb")), [](T x) { return ca::sys::impl::logb(x); }, values, n, mode);
		unary(named("ilogb"), glibc<int(T)>(named("ilogb")), [](T x) { return ca::sys::impl::ilogb(x); }, values, n, mode);
		unary(named("lrint"), glibc<long(T)>(named("lrint")), [](T x) { return static_cast<long>(ca::sys::impl::lrint(x)); }, values, n, mode);
		unary(named("lround"), glibc<long(T)>(named("lround")), [](T x) { return static_cast<long>(ca::sys::impl::lround(x)); }, values, n, mode);
		binary(named("fmod"), glibc<T(T, T)>(named("fmod")), [](T x, T y) { return ca::sys::impl::fmod(x, y); }, values, n, mode);
		zero_sign_free = true;
		binary(named("remainder"), glibc<T(T, T)>(named("remainder")), [](T x, T y) { return ca::sys::impl::remainder(x, y); }, values, n, mode);
		zero_sign_free = false;
		binary(named("fmin"), glibc<T(T, T)>(named("fmin")), [](T x, T y) { return ca::sys::impl::fmin(x, y); }, values, n, mode);
		binary(named("fmax"), glibc<T(T, T)>(named("fmax")), [](T x, T y) { return ca::sys::impl::fmax(x, y); }, values, n, mode);
		binary(named("fdim"), glibc<T(T, T)>(named("fdim")), [](T x, T y) { return ca::sys::impl::fdim(x, y); }, values, n, mode);
		binary(named("nextafter"), glibc<T(T, T)>(named("nextafter")), [](T x, T y) { return ca::sys::impl::nextafter(x, y); }, values, n, mode);
		check_parts(named("remquo"), glibc<T(T, T, int*)>(named("remquo")), glibc<T(T, int*)>(named("frexp")),
		            glibc<T(T, T*)>(named("modf")), values, n, mode);
		check_scaling(named("ldexp"), glibc<T(T, int)>(named("ldexp")), glibc<T(T, long)>(named("scalbln")), values, n, mode);
		if constexpr (sizeof(T) != 16U)
			check_fma(named("fma"), glibc<T(T, T, T)>(named("fma")), values, n, mode);
	}
	set_rounding(0U);
}

auto main(void) -> int {
	libm = dlopen("libm.so.6", 2);
	if (libm == nullptr)
		return 100;
	check_type<float>("f");
	check_type<double>("");
	check_type<long double>("l");
	if (failures != 0)
		printf("%d failures\n", failures);
	return failures == 0 ? 0 : 1;
}
