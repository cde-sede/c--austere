// expect: exit 0
#include "1-primitives/integers/i16.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/i8.hpp"
#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/limits.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/text/parse_integer.hpp"

template <typename T>
static constexpr auto parses(const ca::sv::str text, const T expected, const int base = 10) -> bool {

	auto value = ca::sys::parse_integer<T>(text, base);
	return value && *value == expected;
}

template <typename T>
static constexpr auto fails(const ca::sv::str text, const ca::error error, const int base = 10) -> bool {

	auto value = ca::sys::parse_integer<T>(text, base);
	return !value && value.error() == error;
}

static_assert(parses<int>("0", 0));
static_assert(parses<int>("42", 42));
static_assert(parses<int>("+42", 42));
static_assert(parses<int>("-42", -42));
static_assert(parses<int>("007", 7));
static_assert(parses<ca::i8>("127", 127));
static_assert(parses<ca::i8>("-128", -128));
static_assert(parses<ca::u8>("255", 255U));
static_assert(parses<ca::i16>("-32768", -32768));
static_assert(parses<ca::u16>("ffff", 0xFFFFU, 16));
static_assert(parses<ca::u16>("FfFf", 0xFFFFU, 16));
static_assert(parses<int>("-101", -5, 2));
static_assert(parses<int>("zz", 35 * 36 + 35, 36));
static_assert(parses<ca::i64>("-9223372036854775808", ca::limits<ca::i64>::min()));
static_assert(parses<ca::i64>("9223372036854775807", ca::limits<ca::i64>::max()));
static_assert(parses<ca::u64>("18446744073709551615", ca::limits<ca::u64>::max()));
static_assert(parses<long>("12", 12L));
template <typename T>
concept parsable = requires { ca::sys::parse_integer<T>(ca::sv::str{"1"}); };

static_assert(parsable<unsigned long long>);
static_assert(!parsable<const int>);
static_assert(!parsable<char>);
static_assert(!parsable<char8_t>);
static_assert(!parsable<bool>);

static_assert(fails<int>("", ca::errc::invalid_argument));
static_assert(fails<int>("-", ca::errc::invalid_argument));
static_assert(fails<int>("+", ca::errc::invalid_argument));
static_assert(fails<int>(" 1", ca::errc::invalid_argument));
static_assert(fails<int>("1 ", ca::errc::invalid_argument));
static_assert(fails<int>("1_000", ca::errc::invalid_argument));
static_assert(fails<int>("0x10", ca::errc::invalid_argument, 16));
static_assert(fails<int>("12", ca::errc::invalid_argument, 2));
static_assert(fails<int>("1", ca::errc::invalid_argument, 1));
static_assert(fails<int>("1", ca::errc::invalid_argument, 37));
static_assert(fails<unsigned>("-1", ca::errc::invalid_argument));
static_assert(fails<unsigned>("-0", ca::errc::invalid_argument));
static_assert(fails<ca::i8>("128", ca::errc::out_of_range));
static_assert(fails<ca::i8>("-129", ca::errc::out_of_range));
static_assert(fails<ca::u8>("256", ca::errc::out_of_range));
static_assert(fails<ca::i64>("9223372036854775808", ca::errc::out_of_range));
static_assert(fails<ca::u64>("18446744073709551616", ca::errc::out_of_range));
static_assert(fails<ca::u8>("99999999999999999999x", ca::errc::invalid_argument));

auto main(void) -> int {
	char digits[] = {'-', '3', '1', '4'};
	auto value = ca::sys::parse_integer<int>(ca::sv::str{digits, 4U});
	if (!value || *value != -314)
		return 1;
	auto partial = ca::sys::parse_integer<int>(ca::sv::str{digits, 2U});
	return partial && *partial == -3 ? 0 : 2;
}
