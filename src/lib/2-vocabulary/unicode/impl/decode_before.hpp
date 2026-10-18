#ifndef ca_vocabulary_unicode_impl_decode_before_hpp
#define ca_vocabulary_unicode_impl_decode_before_hpp

#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/unicode/decoded.hpp"
#include "2-vocabulary/unicode/impl/decode_at.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- DECODE BEFORE --------------------------------------------------

		namespace impl {

			/* @DOC The code point ending at at (begin < at), as decoding forward from begin would
			   split it: walking back from the end meets the same code points. */
			template <ca::is_character C>
			[[nodiscard]] constexpr auto decode_before(const C* const begin, const C* const at) noexcept -> ca::unicode::decoded {

				if constexpr (sizeof(C) == 1U) {
					const C* start = at - 1;
					ca::u8 back = 1U;
					while (back < 4U && start > begin && (static_cast<ca::u8>(*start) & 0xC0U) == 0x80U) {
						--start;
						++back;
					}
					const ca::unicode::decoded point = ca::unicode::impl::decode_at(start, at);
					if (point.length() == back)
						return point;
					return ca::unicode::impl::invalid(1U);
				}
				else if constexpr (sizeof(C) == 2U) {
					const ca::u16 last = static_cast<ca::u16>(at[-1]);
					if (last >= 0xDC00U && last <= 0xDFFFU && at - 1 > begin) {
						const ca::u16 first = static_cast<ca::u16>(at[-2]);
						if (first >= 0xD800U && first <= 0xDBFFU)
							return ca::unicode::impl::decode_at(at - 2, at);
					}
					return ca::unicode::impl::decode_at(at - 1, at);
				}
				else
					return ca::unicode::impl::decode_at(at - 1, at);
			}

		} // namespace impl

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_impl_decode_before_hpp
