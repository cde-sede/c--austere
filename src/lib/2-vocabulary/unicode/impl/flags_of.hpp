#ifndef ca_vocabulary_unicode_impl_flags_of_hpp
#define ca_vocabulary_unicode_impl_flags_of_hpp

#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/unicode/impl/char_record.hpp"
#include "2-vocabulary/unicode/impl/tables.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- FLAGS OF -------------------------------------------------------

		namespace impl {

			/* @DOC The class flags of the 128 ASCII code points, one read each, built from the
			   tables at compile time. */
			struct ascii_flags final {

					using self = ca::unicode::impl::ascii_flags;

					ca::u16 _flags[128];

			}; // struct ascii_flags

			[[nodiscard]] consteval auto make_ascii_flags(void) noexcept -> ca::unicode::impl::ascii_flags {

				ca::unicode::impl::ascii_flags table{};
				for (ca::u32 point = 0U; point < 128U; ++point)
					table._flags[point] = ca::unicode::impl::records[ca::unicode::impl::record_index[ca::unicode::impl::block_of[0]][point]]._flags;
				return table;
			}

			inline constexpr ca::unicode::impl::ascii_flags ascii = ca::unicode::impl::make_ascii_flags();

			/* @DOC The class flags of point (char_record::ALPHA, ...): one read for ASCII. */
			[[nodiscard]] constexpr auto flags_of(const char32_t point) noexcept -> ca::u16 {

				const ca::u32 value = static_cast<ca::u32>(point);
				if (value < 0x80U) [[likely]]
					return ca::unicode::impl::ascii._flags[value];
				if (value >= ca::unicode::impl::TABLE_LIMIT)
					return ca::unicode::impl::records[0]._flags;
				const ca::u32 block = ca::unicode::impl::block_of[value >> ca::unicode::impl::BLOCK_SHIFT];
				return ca::unicode::impl::records[ca::unicode::impl::record_index[block][value & ((1U << ca::unicode::impl::BLOCK_SHIFT) - 1U)]]._flags;
			}

		} // namespace impl

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_impl_flags_of_hpp
