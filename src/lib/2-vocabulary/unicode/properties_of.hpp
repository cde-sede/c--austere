#ifndef ca_vocabulary_unicode_properties_of_hpp
#define ca_vocabulary_unicode_properties_of_hpp

#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/unicode/impl/tables.hpp"
#include "2-vocabulary/unicode/properties.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- PROPERTIES OF --------------------------------------------------

		/* @DOC The character database entry of point: two table reads (ASCII), three otherwise,
		   constexpr. A value past the tables (or not a scalar value) has no class and maps to
		   itself. */
		[[nodiscard]] constexpr auto properties_of(const char32_t point) noexcept -> ca::unicode::properties {

			const ca::u32 value = static_cast<ca::u32>(point);
			if (value < 0x80U) [[likely]]
				return ca::unicode::properties{ca::unicode::impl::records[ca::unicode::impl::record_index[ca::unicode::impl::block_of[0]][value]], point};
			if (value >= ca::unicode::impl::TABLE_LIMIT)
				return ca::unicode::properties{ca::unicode::impl::records[0], point};
			const ca::u32 block = ca::unicode::impl::block_of[value >> ca::unicode::impl::BLOCK_SHIFT];
			const ca::u32 slot = value & ((1U << ca::unicode::impl::BLOCK_SHIFT) - 1U);
			return ca::unicode::properties{ca::unicode::impl::records[ca::unicode::impl::record_index[block][slot]], point};
		}

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_properties_of_hpp
