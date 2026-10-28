#ifndef ca_system_text_parse_integer_hpp
#define ca_system_text_parse_integer_hpp

#include "1-primitives/integers/sint.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/numeric/add_overflow.hpp"
#include "1-primitives/numeric/mul_overflow.hpp"
#include "1-primitives/numeric/sub_overflow.hpp"
#include "1-primitives/traits/impl/is_any_of.hpp"
#include "1-primitives/traits/is_integral.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/is_signed.hpp"
#include "1-primitives/traits/remove_cv.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- PARSE INTEGER --------------------------------------------------

		namespace impl {

			[[nodiscard]] constexpr auto digit_value(const char c) noexcept -> ca::sint {

				if (c >= '0' && c <= '9')
					return c - '0';
				if (c >= 'a' && c <= 'z')
					return c - 'a' + 10;
				if (c >= 'A' && c <= 'Z')
					return c - 'A' + 10;
				return 36;
			}

			template <typename T>
			concept is_parsed_integer = ca::is_integral<T> && ca::is_same<T, ca::remove_cv<T>>
				&& !ca::impl::is_any_of<T, bool, char, wchar_t, char8_t, char16_t, char32_t>;

		} // namespace impl


		/* @DOC The integer text spells, the whole of it, in base (2 to 36; letters either case
		   above 9): an optional sign then digits, no space, no prefix such as 0x. '-' only for a
		   signed T. errc::invalid_argument for anything else, errc::out_of_range when the value
		   does not fit in T. T is unqualified. */
		template <ca::sys::impl::is_parsed_integer T>
		[[nodiscard]] constexpr auto parse_integer(const ca::sv::str text, const ca::sint base = 10) noexcept -> ca::result<T> {

			if (base < 2 || base > 36) [[unlikely]]
				return ca::failure{ca::errc::invalid_argument};
			ca::usz at = 0U;
			bool negative = false;
			if (at < text.size() && (text[at] == '+' || text[at] == '-')) {
				negative = text[at] == '-';
				++at;
			}
			if constexpr (!ca::is_signed<T>) {
				if (negative) [[unlikely]]
					return ca::failure{ca::errc::invalid_argument};
			}
			if (at == text.size()) [[unlikely]]
				return ca::failure{ca::errc::invalid_argument};
			T value{0};
			bool overflowed = false;
			for (; at < text.size(); ++at) {
				const ca::sint digit = ca::sys::impl::digit_value(text[at]);
				if (digit >= base) [[unlikely]]
					return ca::failure{ca::errc::invalid_argument};
				if (!overflowed) {
					overflowed = ca::mul_overflow(value, value, base)
						|| (negative ? ca::sub_overflow(value, value, digit) : ca::add_overflow(value, value, digit));
				}
			}
			if (overflowed) [[unlikely]]
				return ca::failure{ca::errc::out_of_range};
			return value;
		}

	} // namespace sys

} // namespace ca

#endif // ca_system_text_parse_integer_hpp
