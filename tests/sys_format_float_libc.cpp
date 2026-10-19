// expect: exit 0
// link: libc
// ca's float text against glibc's snprintf and strtod: every conversion, precisions -1 to 20, #,
// special values and pseudo-random doubles; the shortest form against the smallest %.Ng
// (N >= 15, f32: 6) that reads back. Two documented differences are left out: %#g when rounding
// carries into a new exponent (glibc then drops a digit, C does not), and the shortest form of
// subnormals (ca's is truly the shortest; %.15g prints 15 digits).
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u64.hpp"
#include "3-system/text/format_float.hpp"
#include "3-system/text/format_float_shortest.hpp"

extern "C" auto snprintf(char* out, ca::usz size, const char* format, ...) noexcept -> int;
extern "C" auto strtod(const char* text, char** end) noexcept -> double;
extern "C" auto strtof(const char* text, char** end) noexcept -> float;
extern "C" auto strcmp(const char* a, const char* b) noexcept -> int;

static ca::u64 state = 0x243F6A8885A308D3ULL;

static auto next(void) -> ca::u64 {
	state ^= state << 13U;
	state ^= state >> 7U;
	state ^= state << 17U;
	return state;
}

static auto same(const double value, const char conversion, const int precision, const bool alternate) -> bool {
	char format[8];
	int at = 0;
	format[at++] = '%';
	if (alternate)
		format[at++] = '#';
	format[at++] = '.';
	format[at++] = '*';
	format[at++] = conversion;
	format[at] = 0;
	char want[512];
	char got[512];
	const int shown = precision < 0 && conversion != 'a' && conversion != 'A' ? 6 : precision;
	snprintf(want, sizeof want, format, shown, value);
	ca::sys::format_float(ca::span<char>{got, sizeof got}, value, conversion, shown, alternate);
	return strcmp(want, got) == 0;
}

static auto same_shortest(const double value) -> bool {
	char want[64];
	char got[64];
	for (int digits = 15; digits <= 17; ++digits) {
		snprintf(want, sizeof want, "%.*g", digits, value);
		if (strtod(want, nullptr) == value)
			break;
	}
	ca::sys::format_float_shortest(ca::span<char>{got, sizeof got}, value);
	if (strcmp(want, got) != 0)
		return false;
	const float single = static_cast<float>(value);
	if (single - single != 0.0F || (single < 1.17549435e-38F && single > -1.17549435e-38F))
		return true;
	for (int digits = 6; digits <= 9; ++digits) {
		snprintf(want, sizeof want, "%.*g", digits, static_cast<double>(single));
		if (strtof(want, nullptr) == single)
			break;
	}
	ca::sys::format_float_shortest(ca::span<char>{got, sizeof got}, single);
	return strcmp(want, got) == 0;
}

auto main(void) -> int {
	const char conversions[] = "feEgGaAF";
	const double specials[] = {0.0, -0.0, 1.0, -1.0, 0.5, 1.5, 2.5, 0.05, 0.15, 9.995, 1e-5, 1e-4, 123456789.0, 1e21, 1e22, 1e23, 1e300, 1e-300, 5e-324, 2.2250738585072014e-308, 1.7976931348623157e308, 0.1, 3.14159, 1e16, 999999.4, 1.0 / 0.0, -1.0 / 0.0, 1.9999999999999998, 2.675, 0.125};
	for (const double value : specials) {
		for (const char* c = conversions; *c != 0; ++c) {
			for (int precision = -1; precision <= 20; ++precision) {
				if (!same(value, *c, precision, false))
					return 1;
				if (*c != 'g' && *c != 'G' && !same(value, *c, precision, true))
					return 2;
			}
		}
	}
	for (int i = 0; i < 20000; ++i) {
		const ca::u64 bits = next();
		const double value = __builtin_bit_cast(double, bits);
		const double scaled = static_cast<double>(static_cast<ca::i64>(next() % 2000000001U) - 1000000000) / static_cast<double>(1U + next() % 100000U);
		for (const char* c = conversions; *c != 0; ++c) {
			const int precision = static_cast<int>(next() % 22U) - 1;
			if (!same(value, *c, precision, false) || !same(scaled, *c, precision, false))
				return 3;
		}
		if (value - value == 0.0 && (value >= 2.2250738585072014e-308 || value <= -2.2250738585072014e-308) && !same_shortest(value))
			return 4;
		if (scaled != 0.0 && !same_shortest(scaled))
			return 5;
	}
	return 0;
}
