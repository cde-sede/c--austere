#ifndef ca_collections_map_impl_control_group_hpp
#define ca_collections_map_impl_control_group_hpp

#include "1-primitives/bit/byteswap.hpp"
#include "1-primitives/bit/countr_zero.hpp"
#include "1-primitives/bit/endian.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IMPL ---------------------------------------------------------------

	namespace impl {


		// -- CONTROL GROUP --------------------------------------------------

		/* @DOC Eight control bytes of a hash table, one per slot, matched together in a 64-bit
		   word (SWAR: portable, no vector unit needed). A full slot holds the low 7 bits of its
		   key's hash (high bit clear); EMPTY and DELETED have it set. Masks returned keep the
		   high bit of each matching byte. */
		class control_group final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::control_group;


				// -- private constants --------------------------------------

				static constexpr ca::u64 _lows  = 0x0101010101010101U;
				static constexpr ca::u64 _highs = 0x8080808080808080U;


			public:

				// -- public constants ---------------------------------------

				static constexpr ca::usz WIDTH = 8U;

				static constexpr ca::u8 EMPTY   = 0x80U;
				static constexpr ca::u8 DELETED = 0xFEU;


			private:

				// -- private members ----------------------------------------

				ca::u64 _bytes;


			public:

				// -- public lifecycle ---------------------------------------

				/* @DOC The group at control, eight bytes. */
				[[gnu::always_inline]] explicit control_group(const ca::u8* const control) noexcept
				: _bytes{0U} {

					ca::copy_bytes(&_bytes, control, sizeof(_bytes));
					if constexpr (ca::endian::native == ca::endian::big)
						_bytes = ca::byteswap(_bytes);
				}


				// -- methods ------------------------------------------------

				/* @DOC Full slots holding tag; rarely a false positive next to a true one, never on
				   EMPTY or DELETED. */
				[[nodiscard, gnu::always_inline]] auto match(const ca::u8 tag) const noexcept -> ca::u64 {

					const ca::u64 x = _bytes ^ (_lows * tag);
					return (x - _lows) & ~x & _highs;
				}

				[[nodiscard, gnu::always_inline]] auto match_empty(void) const noexcept -> ca::u64 {
					return _bytes & ~(_bytes << 6U) & _highs;
				}

				[[nodiscard, gnu::always_inline]] auto match_free(void) const noexcept -> ca::u64 {
					return _bytes & ~(_bytes << 7U) & _highs;
				}

				[[nodiscard, gnu::always_inline]] auto match_full(void) const noexcept -> ca::u64 {
					return ~_bytes & _highs;
				}

				/* @DOC Index in the group of the first byte of a non-zero mask. */
				[[nodiscard, gnu::always_inline]] static auto first(const ca::u64 mask) noexcept -> ca::usz {
					return static_cast<ca::usz>(ca::countr_zero(mask)) >> 3U;
				}

		}; // class control_group


		/* @DOC The control bytes of every table owning no storage: one group of EMPTY, so lookups
		   need no branch for it. Never written. */
		alignas(8) inline constinit ca::u8 empty_control[ca::impl::control_group::WIDTH] = {
			0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0x80U,
		};

	} // namespace impl

} // namespace ca

#endif // ca_collections_map_impl_control_group_hpp
