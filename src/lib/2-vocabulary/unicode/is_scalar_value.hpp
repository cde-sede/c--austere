#ifndef ca_vocabulary_unicode_is_scalar_value_hpp
#define ca_vocabulary_unicode_is_scalar_value_hpp

#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- IS SCALAR VALUE ------------------------------------------------

		/* @DOC Whether point is a Unicode scalar value: at most U+10FFFF, not a surrogate
		   (U+D800..U+DFFF). Exactly the code points UTF-8, UTF-16 and UTF-32 can encode. */
		[[nodiscard]] constexpr auto is_scalar_value(const char32_t point) noexcept -> bool {
			return static_cast<ca::u32>(point) <= 0x10FFFFU && (static_cast<ca::u32>(point) < 0xD800U || static_cast<ca::u32>(point) > 0xDFFFU);
		}

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_is_scalar_value_hpp
