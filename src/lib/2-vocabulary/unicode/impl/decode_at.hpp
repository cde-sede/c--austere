#ifndef ca_vocabulary_unicode_impl_decode_at_hpp
#define ca_vocabulary_unicode_impl_decode_at_hpp

#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/unicode/decoded.hpp"
#include "2-vocabulary/unicode/is_scalar_value.hpp"
#include "2-vocabulary/unicode/replacement_character.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- DECODE AT ------------------------------------------------------

		namespace impl {

			[[nodiscard]] constexpr auto invalid(const ca::u8 length) noexcept -> ca::unicode::decoded {
				return ca::unicode::decoded{ca::unicode::replacement_character, length, false};
			}

			template <typename C>
			[[nodiscard]] constexpr auto decode_utf8(const C* const at, const C* const end) noexcept -> ca::unicode::decoded {

				const ca::u32 lead = static_cast<ca::u8>(at[0]);
				if (lead < 0x80U) [[likely]]
					return ca::unicode::decoded{static_cast<char32_t>(lead), 1U, true};
				ca::u32 point = 0U;
				ca::u8 need = 0U;
				ca::u32 low = 0x80U;
				ca::u32 high = 0xBFU;
				if (lead < 0xC2U)
					return ca::unicode::impl::invalid(1U);
				if (lead < 0xE0U) {
					point = lead & 0x1FU;
					 need = 1U;
				}
				else if (lead < 0xF0U) {
					point = lead & 0x0FU;
					 need = 2U;
					  low = lead == 0xE0U ? 0xA0U : 0x80U;
					 high = lead == 0xEDU ? 0x9FU : 0xBFU;
				}
				else if (lead < 0xF5U) {
					point = lead & 0x07U;
					 need = 3U;
					  low = lead == 0xF0U ? 0x90U : 0x80U;
					 high = lead == 0xF4U ? 0x8FU : 0xBFU;
				}
				else
					return ca::unicode::impl::invalid(1U);
				ca::u8 length = 1U;
				while (need != 0U) {
					if (at + length == end)
						return ca::unicode::impl::invalid(length);
					const ca::u32 next = static_cast<ca::u8>(at[length]);
					if (next < low || next > high)
						return ca::unicode::impl::invalid(length);
					point = (point << 6U) | (next & 0x3FU);
					  low = 0x80U;
					 high = 0xBFU;
					++length;
					--need;
				}
				return ca::unicode::decoded{static_cast<char32_t>(point), length, true};
			}

			template <typename C>
			[[nodiscard]] constexpr auto decode_utf16(const C* const at, const C* const end) noexcept -> ca::unicode::decoded {

				const ca::u32 first = static_cast<ca::u16>(at[0]);
				if (first < 0xD800U || first > 0xDFFFU) [[likely]]
					return ca::unicode::decoded{static_cast<char32_t>(first), 1U, true};
				if (first > 0xDBFFU || at + 1 == end)
					return ca::unicode::impl::invalid(1U);
				const ca::u32 second = static_cast<ca::u16>(at[1]);
				if (second < 0xDC00U || second > 0xDFFFU)
					return ca::unicode::impl::invalid(1U);
				return ca::unicode::decoded{static_cast<char32_t>(0x10000U + ((first - 0xD800U) << 10U) + (second - 0xDC00U)), 2U, true};
			}

			/* @DOC The code point at at, before end (at < end), in C's encoding. */
			template <ca::is_character C>
			[[nodiscard]] constexpr auto decode_at(const C* const at, const C* const end) noexcept -> ca::unicode::decoded {

				if constexpr (sizeof(C) == 1U)
					return ca::unicode::impl::decode_utf8(at, end);
				else if constexpr (sizeof(C) == 2U)
					return ca::unicode::impl::decode_utf16(at, end);
				else {
					const char32_t point = static_cast<char32_t>(static_cast<ca::u32>(at[0]));
					if (ca::unicode::is_scalar_value(point)) [[likely]]
						return ca::unicode::decoded{point, 1U, true};
					return ca::unicode::impl::invalid(1U);
				}
			}

		} // namespace impl

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_impl_decode_at_hpp
