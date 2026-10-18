/* @DOC Performance of ca::dynamic_bit_array<A>.

   Layout: a dynamic_array<u64, A> of words (three pointers and the allocator) and the size in
   bits: 32 bytes on 64-bit with ca::heap. Empty arrays own no memory.

   | Operation | Time | Notes |
   |---|---|---|
   | operator[], get, set, reset, flip | O(1) | one compare against size(), a shift and a mask |
   | append, pop | amortized O(1) | a word appended every 64 bits, growth as dynamic_array |
   | resize(n, value) | O(abs(n - size) / 64) | whole words filled at once |
   | count, any, none, all, find_next, ones() walk | O(size / 64) | as bit_array |
   | set_all, reset_all, flip_all, &=, |=, ^=, == | O(size / 64) | |
   | clone | O(size / 64) | one memcpy |

   Space: one bit per element rounded up to 64, plus dynamic_array's growth slack on the words.

   Copy and move: as dynamic_array (clone() is the explicit copy; moves take the block).
   TRIVIALLY_RELOCATABLE when A is.

   Against std::vector<bool>: same packing; whole-word operations (count, find, bitwise) instead
   of a bit at a time through proxy references, which std::vector<bool> does not expose.

   Baseline std::vector<bool>; ca over ca::heap:
   --BENCH bench/bit_array.cpp dynamic_bit_array
   bench/bit_array.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | append 6.5e6 bits | 0.53x | 0.57x | |
   | count 6.5e6 bits, 20 times | 0.011x | 0.041x | popcount per word against a bit loop |
   | operator[] over 6.5e6 bits, 5 times | 0.44x | 0.59x | |
   BENCH-- */

#ifndef ca_collections_array_dynamic_bit_array_hpp
#define ca_collections_array_dynamic_bit_array_hpp

#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/limits.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/array/impl/bit_methods.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DYNAMIC BIT ARRAY --------------------------------------------------

	/* @DOC Growable sequence of bits packed in 64-bit words from A: std::vector<bool> with whole-word
	   operations, boost::dynamic_bitset. The bit operations (test, set, flip, count, find_next,
	   ones, &=, ...) come from ca::impl::bit_methods. append, resize, reserve and clone acquire
	   memory and can fail, changing nothing then. Clean up with destroy(); the destructor is a
	   failsafe. Indexed access traps out of range; get allows the doubt. */
	template <ca::is_allocator A = ca::heap>
	class dynamic_bit_array final : public ca::impl::bit_methods<ca::dynamic_bit_array<A>> {


		private:

			// -- friends ----------------------------------------------------

			friend class ca::impl::bit_methods<ca::dynamic_bit_array<A>>;


			// -- private types ----------------------------------------------

			using self = ca::dynamic_bit_array<A>;


		public:

			// -- public types -----------------------------------------------

			using allocator_type = A;
			using size_type      = ca::usz;

			/* @DOC The same bit array type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::dynamic_bit_array<B>;


			// -- public constants -------------------------------------------

			/* @DOC The words' dynamic_array and a size: containers move it with memcpy when A allows
			   it. */
			static constexpr bool TRIVIALLY_RELOCATABLE = ca::is_trivially_relocatable<A>;


		private:

			// -- private members --------------------------------------------

			ca::dynamic_array<ca::u64, A> _words;
			ca::usz                       _size;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr dynamic_bit_array(void) noexcept requires ca::is_stateless_allocator<A>
			: _words{}, _size{0U} {
			}

			/* @DOC Empty, drawing memory from allocator. */
			constexpr explicit dynamic_bit_array(const A& allocator) noexcept
			: _words{allocator}, _size{0U} {
			}

			dynamic_bit_array(const self&) = delete;

			constexpr dynamic_bit_array(self&& other) noexcept
			: _words{ca::move(other._words)}, _size{ca::exchange(other._size, 0U)} {
			}

			~dynamic_bit_array(void) noexcept = default;

			/* @DOC count bits, each set to value, in memory from allocator. */
			[[nodiscard]] static auto make_in(const A& allocator, const ca::usz count, const bool value = false) noexcept -> ca::result<self> {

				self made{allocator};
				auto sized = made.resize(count, value);
				if (!sized)
					return sized.propagate();
				return made;
			}

			[[nodiscard]] static auto make(const ca::usz count, const bool value = false) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
				return self::make_in(A{}, count, value);
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Takes other's words and allocator; the previous ones are freed. */
			auto operator=(self&& other) noexcept -> self& {

				if (this == &other)
					return *this;
				_words = ca::move(other._words);
				 _size = ca::exchange(other._size, 0U);
				return *this;
			}


			// -- accessors --------------------------------------------------

			/* @DOC Number of bits. */
			[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
				return _size;
			}

			/* @DOC Bits held without growing. */
			[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
				return _words.capacity() * ca::impl::bit_word_size;
			}

			[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
				return _size == 0U;
			}

			/* @DOC The words, bit i at word i / 64, position i % 64; bits past size() are 0. */
			[[nodiscard]] constexpr auto words(void) const noexcept -> ca::borrowed_slice<const ca::u64> {
				return _words.view();
			}

			[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
				return _words.allocator();
			}


			// -- modifiers --------------------------------------------------

			/* @DOC Makes room for count bits in total; never shrinks. On failure nothing changes. */
			[[nodiscard]] auto reserve(const ca::usz count) noexcept -> ca::result<void> {
				return _words.reserve(ca::impl::bit_words_for(count));
			}

			/* @DOC Sets size() to count, new bits set to value. On failure nothing changes. */
			[[nodiscard]] auto resize(const ca::usz count, const bool value = false) noexcept -> ca::result<void> {

				if (count > ca::limits<ca::usz>::max() - ca::impl::bit_word_size) [[unlikely]]
					return ca::failure{ca::errc::out_of_memory};
				const ca::usz words = ca::impl::bit_words_for(count);
				if (count > _size) {
					auto grown = _words.resize(words, value ? ~ca::u64{0U} : ca::u64{0U});
					if (!grown)
						return grown;
					const ca::usz used = _size % ca::impl::bit_word_size;
					if (value && used != 0U)
						_words.data()[_size / ca::impl::bit_word_size] |= ~ca::u64{0U} << used;
				}
				else
					static_cast<void>(_words.resize(words));
				_size = count;
				this->_clear_tail();
				return ca::result<void>{};
			}

			/* @DOC Appends value. On failure nothing changes. */
			[[nodiscard]] auto append(const bool value) noexcept -> ca::result<void> {

				if (_size % ca::impl::bit_word_size == 0U) {
					auto added = _words.append(ca::u64{0U});
					if (!added) [[unlikely]]
						return added;
				}
				if (value)
					_words.data()[_size / ca::impl::bit_word_size] |= ca::u64{1U} << (_size % ca::impl::bit_word_size);
				++_size;
				return ca::result<void>{};
			}

			/* @DOC Removes and returns the last bit, or nothing when empty. */
			[[nodiscard]] auto pop(void) noexcept -> ca::optional<bool> {

				if (_size == 0U)
					return ca::nullopt;
				const bool last = (*this)[_size - 1U];
				this->reset(_size - 1U);
				--_size;
				if (_size % ca::impl::bit_word_size == 0U)
					static_cast<void>(_words.pop());
				return last;
			}

			/* @DOC Removes every bit; keeps the capacity. */
			auto clear(void) noexcept -> void {

				static_cast<void>(_words.clear());
				_size = 0U;
			}

			/* @DOC Frees the words: empty, no capacity. */
			[[nodiscard]] auto destroy(void) noexcept -> ca::result<void> {

				_size = 0U;
				return _words.destroy();
			}


			// -- methods ----------------------------------------------------

			[[nodiscard]] constexpr auto operator==(const self& other) const noexcept -> bool {
				return _size == other._size && _words.view() == other._words.view();
			}

			/* @DOC A copy in memory from the same allocator. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self> {
				return this->clone_in(_words.allocator());
			}

			/* @DOC clone, in memory from allocator. */
			[[nodiscard]] auto clone_in(const A& allocator) const noexcept -> ca::result<self> {

				auto words = _words.clone_in(allocator);
				if (!words)
					return words.propagate();
				self copy{allocator};
				copy._words = ca::move(*words);
				 copy._size = _size;
				return copy;
			}


		private:

			// -- private methods --------------------------------------------

			[[nodiscard]] constexpr auto _mutable_words(void) noexcept -> ca::borrowed_slice<ca::u64> {
				return _words.view();
			}

			constexpr auto _clear_tail(void) noexcept -> void {

				const ca::usz used = _size % ca::impl::bit_word_size;
				if (used != 0U)
					_words.data()[_words.size() - 1U] &= (ca::u64{1U} << used) - 1U;
			}

	}; // class dynamic_bit_array

} // namespace ca

#endif // ca_collections_array_dynamic_bit_array_hpp
