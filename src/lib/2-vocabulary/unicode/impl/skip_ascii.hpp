#ifndef ca_vocabulary_unicode_impl_skip_ascii_hpp
#define ca_vocabulary_unicode_impl_skip_ascii_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/traits/is_constant_evaluated.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- SKIP ASCII -----------------------------------------------------

		namespace impl {

			/* @DOC First position from at, 16 bytes at a time, where a byte >= 0x80 may be; at
			   itself in constant evaluation. 1-byte units only. */
			template <typename C>
			[[nodiscard]] constexpr auto skip_ascii(const C* at, const C* const end) noexcept -> const C* {

				if (ca::is_constant_evaluated())
					return at;
				while (end - at >= 16) {
					ca::u64 low{0U};
					ca::u64 high{0U};
					ca::copy_bytes(&low, at, 8U);
					ca::copy_bytes(&high, at + 8, 8U);
					if (((low | high) & 0x8080808080808080U) != 0U)
						break;
					at += 16;
				}
				return at;
			}

			/* @DOC Whether the 16 bytes at at are all ASCII. Not in constant evaluation. */
			template <typename C>
			[[nodiscard]] inline auto is_ascii_block(const C* const at) noexcept -> bool {

				ca::u64 low{0U};
				ca::u64 high{0U};
				ca::copy_bytes(&low, at, 8U);
				ca::copy_bytes(&high, at + 8, 8U);
				return ((low | high) & 0x8080808080808080U) == 0U;
			}

			/* @DOC Whether the count bytes at at are all ASCII: words and overlapping loads, few
			   branches. Not in constant evaluation. */
			template <typename C>
			[[nodiscard]] inline auto is_ascii(const C* const at, const ca::usz count) noexcept -> bool {

				if (count >= 8U) {
					ca::u64 bits{0U};
					ca::u64 word{0U};
					for (ca::usz i = 0U; i + 8U <= count; i += 8U) {
						ca::copy_bytes(&word, at + i, 8U);
						bits |= word;
					}
					ca::copy_bytes(&word, at + count - 8U, 8U);
					return ((bits | word) & 0x8080808080808080U) == 0U;
				}
				if (count >= 4U) {
					ca::u32 first{0U};
					ca::u32 last{0U};
					ca::copy_bytes(&first, at, 4U);
					ca::copy_bytes(&last, at + count - 4U, 4U);
					return ((first | last) & 0x80808080U) == 0U;
				}
				ca::u8 bits{0U};
				for (ca::usz i = 0U; i < count; ++i)
					bits = static_cast<ca::u8>(bits | static_cast<ca::u8>(at[i]));
				return (bits & 0x80U) == 0U;
			}

		} // namespace impl

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_impl_skip_ascii_hpp
