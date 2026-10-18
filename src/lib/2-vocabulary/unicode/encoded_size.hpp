#ifndef ca_vocabulary_unicode_encoded_size_hpp
#define ca_vocabulary_unicode_encoded_size_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/unicode/is_scalar_value.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- ENCODED SIZE ---------------------------------------------------

		/* @DOC Code units point takes in C's encoding; a value that is not a scalar value counts
		   as U+FFFD. */
		template <ca::is_character C>
		[[nodiscard]] constexpr auto encoded_size(const char32_t point) noexcept -> ca::usz {

			const ca::u32 value = ca::unicode::is_scalar_value(point) ? static_cast<ca::u32>(point) : 0xFFFDU;
			if constexpr (sizeof(C) == 1U)
				return value < 0x80U ? 1U : value < 0x800U ? 2U : value < 0x10000U ? 3U : 4U;
			else if constexpr (sizeof(C) == 2U)
				return value < 0x10000U ? 1U : 2U;
			else
				return 1U;
		}

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_encoded_size_hpp
