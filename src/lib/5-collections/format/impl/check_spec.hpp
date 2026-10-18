#ifndef ca_collections_format_impl_check_spec_hpp
#define ca_collections_format_impl_check_spec_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "5-collections/format/impl/display_kind.hpp"
#include "5-collections/format/impl/format_errors.hpp"
#include "5-collections/format/impl/format_spec.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- CHECK SPEC ---------------------------------------------------------

	namespace impl {

		inline constexpr ca::usz max_width = 4096U;

		inline constexpr ca::usz max_precision = 200U;

		[[nodiscard]] consteval auto kind_bit(const ca::impl::display_kind kind) noexcept -> ca::u32 {
			return 1U << static_cast<ca::u32>(kind);
		}

		/* @DOC Rejects, at compile time, a spec the argument kind cannot honour. */
		consteval auto check_spec(const ca::impl::format_spec& spec, const ca::impl::display_kind kind) noexcept -> void {

			using k = ca::impl::display_kind;
			constexpr ca::u32 numbers  = ca::impl::kind_bit(k::character) | ca::impl::kind_bit(k::code_point) | ca::impl::kind_bit(k::signed_integer)
				| ca::impl::kind_bit(k::unsigned_integer);
			constexpr ca::u32 integers = numbers | ca::impl::kind_bit(k::boolean);
			constexpr ca::u32 strings  = ca::impl::kind_bit(k::string) | ca::impl::kind_bit(k::wide_string);
			constexpr ca::u32 quotable = strings | ca::impl::kind_bit(k::character) | ca::impl::kind_bit(k::code_point);
			constexpr ca::u32 texts    = quotable | ca::impl::kind_bit(k::boolean) | ca::impl::kind_bit(k::error);
			constexpr ca::u32 precise  = strings | ca::impl::kind_bit(k::signed_integer) | ca::impl::kind_bit(k::unsigned_integer)
				| ca::impl::kind_bit(k::floating);
			const ca::u32 bit = ca::impl::kind_bit(kind);

			if (kind == k::custom) {
				if (spec._width != 0U || spec._precision >= 0 || spec._align != '\0' || spec._sign != '\0' || spec._alternate || spec._zero
				 || (spec._type != '\0' && spec._type != 's'))
					ca::impl::format_custom_display_takes_no_spec();
				return;
			}
			ca::u32 fitting = 0U;
			switch (spec._type) {
				case '\0':
					fitting = bit;
					break;
				case 'd':
					fitting = integers | ca::impl::kind_bit(k::error);
					break;
				case 'x': case 'X': case 'o': case 'b': case 'B':
					fitting = integers;
					break;
				case 'c':
					fitting = numbers;
					break;
				case 's':
					fitting = texts;
					break;
				case 'p':
					fitting = ca::impl::kind_bit(k::pointer) | strings;
					break;
				case '?':
					fitting = quotable;
					break;
				case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': case 'a': case 'A':
					fitting = ca::impl::kind_bit(k::floating);
					break;
				default:
					break;
			}
			if ((fitting & bit) == 0U)
				ca::impl::format_type_does_not_fit_the_argument();
			if (spec._precision >= 0 && ((precise & bit) == 0U || spec._type == 'c' || spec._type == 'p'))
				ca::impl::format_precision_does_not_fit_the_argument();
		}

		/* @DOC Reads decimal digits at text[at], advancing at; fails to compile past max. */
		consteval auto parse_number(const char* const text, ca::usz& at, const ca::usz max) noexcept -> ca::usz {

			ca::usz value = 0U;
			while (text[at] >= '0' && text[at] <= '9') {
				value = value * 10U + static_cast<ca::usz>(text[at] - '0');
				if (value > max)
					ca::impl::format_width_or_precision_is_too_large();
				++at;
			}
			return value;
		}

	} // namespace impl

} // namespace ca

#endif // ca_collections_format_impl_check_spec_hpp
