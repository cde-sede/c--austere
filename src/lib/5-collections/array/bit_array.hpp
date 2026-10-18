/* @DOC Performance of ca::bit_array<N>.

   Layout: ceil(N / 64) words of 64 bits in the object itself, nothing else: bit_array<100> is 16
   bytes, bit_array<512> 64. No pointer, no size, no allocation; trivially copyable, usable in
   constant expressions.

   | Operation | Time | Notes |
   |---|---|---|
   | operator[], get, set, reset, flip | O(1) | one compare against N, a shift and a mask |
   | count | O(N / 64) | one popcount per word (popcnt with -mpopcnt or -march) |
   | any, none, all | O(N / 64) | stops at the first word that decides |
   | find_first, find_next, and their unset forms | O(N / 64) | one countr_zero on the first non-empty word |
   | ones() walk | O(N / 64 + set bits) | clears the lowest bit per step |
   | set_all, reset_all, flip_all, &=, |=, ^=, == | O(N / 64) | word loops; vectorize |

   Space: N bits rounded up to 64. Locality: one contiguous run of words.

   Against std::bitset: same layout and complexities; adds find_next/find_next_unset (std has
   none; libstdc++'s _Find_next is an extension), the ones() range, traps on bad indices in every
   build, and allocation-free constexpr throughout.

   Baseline std::bitset<65536>; ca is bit_array<65536>:
   --BENCH bench/bit_array.cpp bit_array
   bench/bit_array.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | count 64 Ki bits, 2e3 times | 1.03x | 1.04x | |
   | scan a sparse set with find_first/find_next, 2e2 times | 1.16x | 1.00x | std: libstdc++'s _Find_first/_Find_next |
   | set every 61st bit, 2e3 times | 1.00x | 1.00x | |
   BENCH-- */

#ifndef ca_collections_array_bit_array_hpp
#define ca_collections_array_bit_array_hpp

#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "5-collections/array/impl/bit_methods.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BIT ARRAY ----------------------------------------------------------

	/* @DOC N bits packed in 64-bit words held in the object itself: std::bitset. Starts all clear.
	   The bit operations (test, set, flip, count, find_next, ones, &=, ...) come from
	   ca::impl::bit_methods; indexed access traps out of range, get allows the doubt.
	   Trivially copyable and constexpr. */
	template <ca::usz N>
	class bit_array final : public ca::impl::bit_methods<ca::bit_array<N>> {


		private:

			// -- friends ----------------------------------------------------

			friend class ca::impl::bit_methods<ca::bit_array<N>>;


			// -- private types ----------------------------------------------

			using self = ca::bit_array<N>;


			// -- private constants ------------------------------------------

			static constexpr ca::usz _word_count = ca::impl::bit_words_for(N);


		public:

			// -- public types -----------------------------------------------

			using size_type = ca::usz;


			// -- public constants -------------------------------------------

			/* @DOC Number of bits. */
			static constexpr ca::usz SIZE = N;


		private:

			// -- private members --------------------------------------------

			ca::u64 _words[_word_count];


		public:

			// -- public lifecycle -------------------------------------------

			/* @DOC Every bit clear. */
			constexpr bit_array(void) noexcept
			: _words{} {
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] static constexpr auto size(void) noexcept -> ca::usz {
				return N;
			}

			/* @DOC The words, bit i at word i / 64, position i % 64; bits past N are 0. */
			[[nodiscard]] constexpr auto words(void) const noexcept -> ca::borrowed_slice<const ca::u64> {
				return ca::borrowed_slice<const ca::u64>{_words, _word_count};
			}


			// -- methods ----------------------------------------------------

			[[nodiscard]] constexpr auto operator==(const self& other) const noexcept -> bool {

				for (ca::usz i = 0U; i < _word_count; ++i) {
					if (_words[i] != other._words[i])
						return false;
				}
				return true;
			}


		private:

			// -- private methods --------------------------------------------

			[[nodiscard]] constexpr auto _mutable_words(void) noexcept -> ca::borrowed_slice<ca::u64> {
				return ca::borrowed_slice<ca::u64>{_words, _word_count};
			}


			// -- assertions -------------------------------------------------

			static_assert(N != 0U, "ca::bit_array<N>: N must not be 0");

	}; // class bit_array

} // namespace ca

#endif // ca_collections_array_bit_array_hpp
