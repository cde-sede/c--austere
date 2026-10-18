#ifndef ca_vocabulary_sv_impl_classify_hpp
#define ca_vocabulary_sv_impl_classify_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/uint_of.hpp"
#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/unicode/decoded.hpp"
#include "2-vocabulary/unicode/impl/decode_at.hpp"
#include "2-vocabulary/unicode/impl/decode_before.hpp"
#include "2-vocabulary/unicode/properties.hpp"
#include "2-vocabulary/unicode/properties_of.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SV -----------------------------------------------------------------

	namespace sv {


		// -- CLASSIFY -------------------------------------------------------

		namespace impl {

			/* @DOC The code point at at (at < end) with its length; ASCII without decoding. */
			template <ca::is_character C>
			[[nodiscard]] constexpr auto point_at(const C* const at, const C* const end) noexcept -> ca::unicode::decoded {

				const ca::u32 unit = static_cast<ca::uint_of<sizeof(C) * 8U>>(*at);
				if (unit < 0x80U) [[likely]]
					return ca::unicode::decoded{static_cast<char32_t>(unit), 1U, true};
				return ca::unicode::impl::decode_at(at, end);
			}

			/* @DOC The code point ending at at (begin < at); ASCII without decoding. */
			template <ca::is_character C>
			[[nodiscard]] constexpr auto point_before(const C* const begin, const C* const at) noexcept -> ca::unicode::decoded {

				const ca::u32 unit = static_cast<ca::uint_of<sizeof(C) * 8U>>(at[-1]);
				if (unit < 0x80U) [[likely]]
					return ca::unicode::decoded{static_cast<char32_t>(unit), 1U, true};
				return ca::unicode::impl::decode_before(begin, at);
			}

			/* @DOC Units of the white space code point at at, 0 when it is not one. */
			template <ca::is_character C>
			[[nodiscard]] constexpr auto space_at(const C* const at, const C* const end) noexcept -> ca::u8 {

				const ca::unicode::decoded point = ca::sv::impl::point_at(at, end);
				return point.valid() && ca::unicode::properties_of(point.code_point()).is_space() ? point.length() : ca::u8{0U};
			}

			/* @DOC Units of the white space code point ending at at, 0 when it is not one. */
			template <ca::is_character C>
			[[nodiscard]] constexpr auto space_before(const C* const begin, const C* const at) noexcept -> ca::u8 {

				const ca::unicode::decoded point = ca::sv::impl::point_before(begin, at);
				return point.valid() && ca::unicode::properties_of(point.code_point()).is_space() ? point.length() : ca::u8{0U};
			}

			/* @DOC Units of the line boundary at at ("\r\n" is one), 0 when there is none. */
			template <ca::is_character C>
			[[nodiscard]] constexpr auto line_break_at(const C* const at, const C* const end) noexcept -> ca::u8 {

				const ca::unicode::decoded point = ca::sv::impl::point_at(at, end);
				if (!point.valid() || !ca::unicode::properties_of(point.code_point()).is_line_break())
					return 0U;
				if (point.code_point() == U'\r' && at + 1 != end && at[1] == C{'\n'})
					return 2U;
				return point.length();
			}

		} // namespace impl

	} // namespace sv

} // namespace ca

#endif // ca_vocabulary_sv_impl_classify_hpp
