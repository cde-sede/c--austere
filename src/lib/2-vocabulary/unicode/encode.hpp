#ifndef ca_vocabulary_unicode_encode_hpp
#define ca_vocabulary_unicode_encode_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/unicode/is_scalar_value.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- ENCODE ---------------------------------------------------------

		/* @DOC Writes point at out in C's encoding, room for max_units<C> needed, and returns the
		   units written. A value that is not a scalar value is written as U+FFFD. */
		template <ca::is_character C>
		constexpr auto encode(const char32_t point, C* const out) noexcept -> ca::usz {

			const ca::u32 value = ca::unicode::is_scalar_value(point) ? static_cast<ca::u32>(point) : 0xFFFDU;
			if constexpr (sizeof(C) == 1U) {
				if (value < 0x80U) {
					out[0] = static_cast<C>(value);
					return 1U;
				}
				if (value < 0x800U) {
					out[0] = static_cast<C>(0xC0U | (value >> 6U));
					out[1] = static_cast<C>(0x80U | (value & 0x3FU));
					return 2U;
				}
				if (value < 0x10000U) {
					out[0] = static_cast<C>(0xE0U | (value >> 12U));
					out[1] = static_cast<C>(0x80U | ((value >> 6U) & 0x3FU));
					out[2] = static_cast<C>(0x80U | (value & 0x3FU));
					return 3U;
				}
				out[0] = static_cast<C>(0xF0U | (value >> 18U));
				out[1] = static_cast<C>(0x80U | ((value >> 12U) & 0x3FU));
				out[2] = static_cast<C>(0x80U | ((value >> 6U) & 0x3FU));
				out[3] = static_cast<C>(0x80U | (value & 0x3FU));
				return 4U;
			}
			else if constexpr (sizeof(C) == 2U) {
				if (value < 0x10000U) {
					out[0] = static_cast<C>(value);
					return 1U;
				}
				out[0] = static_cast<C>(0xD800U + ((value - 0x10000U) >> 10U));
				out[1] = static_cast<C>(0xDC00U + ((value - 0x10000U) & 0x3FFU));
				return 2U;
			}
			else {
				out[0] = static_cast<C>(value);
				return 1U;
			}
		}

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_encode_hpp
