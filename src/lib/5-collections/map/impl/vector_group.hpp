#ifndef ca_collections_map_impl_vector_group_hpp
#define ca_collections_map_impl_vector_group_hpp

#include "1-primitives/bit/countr_zero.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/simd/load.hpp"
#include "1-primitives/simd/mask_of.hpp"
#include "1-primitives/simd/splat.hpp"
#include "1-primitives/simd/vec.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IMPL ---------------------------------------------------------------

	namespace impl {


		// -- VECTOR GROUP ---------------------------------------------------

		/* @DOC Sixteen control bytes of a hash table, one per slot, matched together in a 16-byte
		   vector (SSE2: one compare and one pmovmskb). The bytes as swar_group: a full slot holds
		   the low 7 bits of its key's hash, EMPTY and DELETED have the high bit set, so the free
		   slots are the high bits themselves. Masks returned hold bit i for slot i. */
		class vector_group final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::vector_group;

				using lanes = ca::simd::vec<char, 16U>;


			public:

				// -- public types -------------------------------------------

				/* @DOC A set of slots of the group, as the matches return it. */
				using bits = ca::u32;


				// -- public constants ---------------------------------------

				static constexpr ca::usz WIDTH = 16U;
				static constexpr ca::usz SHIFT = 4U;

				static constexpr ca::u8 EMPTY   = 0x80U;
				static constexpr ca::u8 DELETED = 0xFEU;


			private:

				// -- private members ----------------------------------------

				lanes _bytes;


			public:

				// -- public lifecycle ---------------------------------------

				/* @DOC The group at control, sixteen bytes, any alignment. */
				[[gnu::always_inline]] explicit vector_group(const ca::u8* const control) noexcept
				: _bytes{} {
					ca::simd::load(control, _bytes);
				}


				// -- methods ------------------------------------------------

				/* @DOC Full slots holding tag; never EMPTY or DELETED, whose high bit tag lacks. */
				[[nodiscard, gnu::always_inline]] auto match(const ca::u8 tag) const noexcept -> bits {

					lanes needle;
					ca::simd::splat(tag, needle);
					return ca::simd::mask_of(_bytes == needle).bits();
				}

				[[nodiscard, gnu::always_inline]] auto match_empty(void) const noexcept -> bits {

					lanes empty;
					ca::simd::splat(EMPTY, empty);
					return ca::simd::mask_of(_bytes == empty).bits();
				}

				[[nodiscard, gnu::always_inline]] auto match_free(void) const noexcept -> bits {
					return ca::simd::mask_of(_bytes).bits();
				}

				[[nodiscard, gnu::always_inline]] auto match_full(void) const noexcept -> bits {
					return ca::simd::mask_of(_bytes).bits() ^ 0xFFFFU;
				}

				/* @DOC Index in the group of the first slot of a non-empty set. */
				[[nodiscard, gnu::always_inline]] static auto first(const bits set) noexcept -> ca::usz {
					return static_cast<ca::usz>(ca::countr_zero(set));
				}

				/* @DOC The slots of set from index lane of the group on. */
				[[nodiscard, gnu::always_inline]] static auto from(const bits set, const ca::usz lane) noexcept -> bits {
					return set & (~bits{0U} << lane);
				}

		}; // class vector_group

	} // namespace impl

} // namespace ca

#endif // ca_collections_map_impl_vector_group_hpp
