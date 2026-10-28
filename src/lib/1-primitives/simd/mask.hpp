#ifndef ca_primitives_simd_mask_hpp
#define ca_primitives_simd_mask_hpp

#include "1-primitives/bit/countl_zero.hpp"
#include "1-primitives/bit/countr_zero.hpp"
#include "1-primitives/bit/popcount.hpp"
#include "1-primitives/integers/bits.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/conditional.hpp"


// -- CA SIMD NAMESPACE ------------------------------------------------------

namespace ca::simd {


	// -- MASK ---------------------------------------------------------------

	/* @DOC One bit per lane of a vector of N lanes, bit i for lane i: what ca::simd::mask_of
	   makes of a comparison. In a 32-bit word up to 32 lanes, else 64; the bits from N are always
	   clear. `for (ca::usz lane : m)` walks the set lanes in increasing order. */
	template <ca::usz N>
	requires (N >= 1U && N <= 64U)
	class mask final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::simd::mask<N>;


		public:

			// -- public types -----------------------------------------------

			using word = ca::conditional<(N <= 32U), ca::u32, ca::u64>;


			/* @DOC Walks the set lanes of a mask, in increasing order. */
			class iterator final {


				private:

					// -- private types ------------------------------------------

					using self = typename ca::simd::mask<N>::iterator;


				private:

					// -- private members ----------------------------------------

					word _rest;


				public:

					// -- public lifecycle ---------------------------------------

					constexpr explicit iterator(const word rest) noexcept
					: _rest{rest} {
					}


					// -- element access -----------------------------------------

					/* @DOC The current lane. */
					[[nodiscard, gnu::always_inline]] constexpr auto operator*(void) const noexcept -> ca::usz {
						return static_cast<ca::usz>(ca::countr_zero(_rest));
					}


					// -- methods ------------------------------------------------

					[[gnu::always_inline]] constexpr auto operator++(void) noexcept -> self& {

						_rest &= _rest - 1U;
						return *this;
					}

					[[nodiscard, gnu::always_inline]] constexpr auto operator==(const self& other) const noexcept -> bool {
						return _rest == other._rest;
					}

			}; // class iterator


			// -- public constants -------------------------------------------

			static constexpr ca::usz LANES = N;


		private:

			// -- private constants ------------------------------------------

			static constexpr word _all = N == ca::bits<word> ? ~word{0U} : (word{1U} << N) - 1U;


			// -- private members --------------------------------------------

			word _bits;


		public:

			// -- public lifecycle -------------------------------------------

			/* @DOC No lane set. */
			constexpr mask(void) noexcept
			: _bits{0U} {
			}

			/* @DOC Lane i set when bit i of bits is; the bits from N must be clear. */
			[[gnu::always_inline]] constexpr explicit mask(const word bits) noexcept
			: _bits{bits} {
			}

			/* @DOC Every lane set. */
			[[nodiscard]] static constexpr auto full(void) noexcept -> self {
				return self{_all};
			}


			// -- iterators --------------------------------------------------

			[[nodiscard, gnu::always_inline]] constexpr auto begin(void) const noexcept -> iterator {
				return iterator{_bits};
			}

			[[nodiscard, gnu::always_inline]] constexpr auto end(void) const noexcept -> iterator {
				return iterator{0U};
			}


			// -- accessors --------------------------------------------------

			/* @DOC The lanes as a word, bit i for lane i. */
			[[nodiscard, gnu::always_inline]] constexpr auto bits(void) const noexcept -> word {
				return _bits;
			}

			[[nodiscard, gnu::always_inline]] constexpr auto any(void) const noexcept -> bool {
				return _bits != 0U;
			}

			[[nodiscard, gnu::always_inline]] constexpr auto none(void) const noexcept -> bool {
				return _bits == 0U;
			}

			[[nodiscard, gnu::always_inline]] constexpr auto all(void) const noexcept -> bool {
				return _bits == _all;
			}

			/* @DOC Whether lane is set; false from N. */
			[[nodiscard, gnu::always_inline]] constexpr auto test(const ca::usz lane) const noexcept -> bool {
				return lane < N && ((_bits >> lane) & 1U) != 0U;
			}

			/* @DOC Set lanes. */
			[[nodiscard, gnu::always_inline]] constexpr auto count(void) const noexcept -> ca::usz {
				return static_cast<ca::usz>(ca::popcount(_bits));
			}

			/* @DOC The lowest set lane; at least N when none is. */
			[[nodiscard, gnu::always_inline]] constexpr auto first(void) const noexcept -> ca::usz {
				return static_cast<ca::usz>(ca::countr_zero(_bits));
			}

			/* @DOC The highest set lane; at least N when none is. */
			[[nodiscard, gnu::always_inline]] constexpr auto last(void) const noexcept -> ca::usz {
				return ca::bits<word> - 1U - static_cast<ca::usz>(ca::countl_zero(_bits));
			}


			// -- methods ----------------------------------------------------

			/* @DOC The set lanes below count, all of them from N. */
			[[nodiscard, gnu::always_inline]] constexpr auto below(const ca::usz count) const noexcept -> self {
				return count >= N ? *this : self{static_cast<word>(_bits & ((word{1U} << count) - 1U))};
			}

			/* @DOC The set lanes from count, none from N. */
			[[nodiscard, gnu::always_inline]] constexpr auto from(const ca::usz count) const noexcept -> self {
				return count >= N ? self{} : self{static_cast<word>(_bits & ~((word{1U} << count) - 1U))};
			}

			[[nodiscard, gnu::always_inline]] constexpr auto operator&(const self other) const noexcept -> self {
				return self{static_cast<word>(_bits & other._bits)};
			}

			[[nodiscard, gnu::always_inline]] constexpr auto operator|(const self other) const noexcept -> self {
				return self{static_cast<word>(_bits | other._bits)};
			}

			[[nodiscard, gnu::always_inline]] constexpr auto operator^(const self other) const noexcept -> self {
				return self{static_cast<word>(_bits ^ other._bits)};
			}

			/* @DOC The lanes not set, of the N. */
			[[nodiscard, gnu::always_inline]] constexpr auto operator~(void) const noexcept -> self {
				return self{static_cast<word>(_bits ^ _all)};
			}

			[[gnu::always_inline]] constexpr auto operator&=(const self other) noexcept -> self& {

				_bits &= other._bits;
				return *this;
			}

			[[gnu::always_inline]] constexpr auto operator|=(const self other) noexcept -> self& {

				_bits |= other._bits;
				return *this;
			}

			[[gnu::always_inline]] constexpr auto operator^=(const self other) noexcept -> self& {

				_bits ^= other._bits;
				return *this;
			}

			[[nodiscard, gnu::always_inline]] constexpr auto operator==(const self other) const noexcept -> bool {
				return _bits == other._bits;
			}

	}; // class mask

} // namespace ca::simd

#endif // ca_primitives_simd_mask_hpp
