#ifndef ca_collections_array_impl_bit_methods_hpp
#define ca_collections_array_impl_bit_methods_hpp

#include "1-primitives/bit/countr_zero.hpp"
#include "1-primitives/bit/popcount.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BIT METHODS --------------------------------------------------------

	namespace impl {

		/* @DOC Bits per word of a bit array. */
		inline constexpr ca::usz bit_word_size = 64U;

		/* @DOC Words holding count bits. */
		[[nodiscard]] constexpr auto bit_words_for(const ca::usz count) noexcept -> ca::usz {
			return count / ca::impl::bit_word_size + (count % ca::impl::bit_word_size != 0U ? 1U : 0U);
		}


		/* @DOC Walks the indices of the set bits of a bit array, in increasing order. */
		class set_bit_iterator final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::set_bit_iterator;


			private:

				// -- private members ----------------------------------------

				const ca::u64* _words;
				ca::usz        _count;
				ca::usz        _index;
				ca::u64        _current;


			public:

				// -- public lifecycle ---------------------------------------

				/* @DOC At the first set bit of the count words from words at or after word index. */
				constexpr set_bit_iterator(const ca::u64* const words, const ca::usz count, const ca::usz index) noexcept
				: _words{words}, _count{count}, _index{index}, _current{index < count ? words[index] : 0U} {
					this->_skip_empty();
				}


				// -- element access -----------------------------------------

				/* @DOC Index of the current set bit. */
				[[nodiscard]] constexpr auto operator*(void) const noexcept -> ca::usz {
					return _index * ca::impl::bit_word_size + static_cast<ca::usz>(ca::countr_zero(_current));
				}


				// -- methods ------------------------------------------------

				constexpr auto operator++(void) noexcept -> self& {

					_current &= _current - 1U;
					this->_skip_empty();
					return *this;
				}

				[[nodiscard]] constexpr auto operator==(const self& other) const noexcept -> bool {
					return _index == other._index && _current == other._current;
				}


			private:

				// -- private methods ----------------------------------------

				constexpr auto _skip_empty(void) noexcept -> void {
					while (_current == 0U && _index < _count) {
						++_index;
						_current = _index < _count ? _words[_index] : 0U;
					}
				}

		}; // class set_bit_iterator


		/* @DOC The indices of the set bits of a bit array: `for (ca::usz i : bits.ones())`. Valid
		   while the array is unchanged. */
		class set_bits final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::set_bits;


				// -- private members ----------------------------------------

				const ca::u64* _words;
				ca::usz        _count;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr set_bits(const ca::u64* const words, const ca::usz count) noexcept
				: _words{words}, _count{count} {
				}


				// -- iterators ----------------------------------------------

				[[nodiscard]] constexpr auto begin(void) const noexcept -> ca::impl::set_bit_iterator {
					return ca::impl::set_bit_iterator{_words, _count, 0U};
				}

				[[nodiscard]] constexpr auto end(void) const noexcept -> ca::impl::set_bit_iterator {
					return ca::impl::set_bit_iterator{_words, _count, _count};
				}

		}; // class set_bits


		/* @DOC The bit operations, written once for bit_array<N> and dynamic_bit_array<A>. D gives
		   size() in bits, words() (a borrowed_slice<const u64>, bit i at word i / 64, bit i % 64)
		   and a private _mutable_words(). Bits past size() in the last word are always 0, so counts
		   and searches need no mask. Indexed access traps out of range, in every build. */
		template <typename D> // final: ignore
		class bit_methods {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::bit_methods<D>;


			public:

				// -- element access -----------------------------------------

				/* @DOC Bit at index. Traps when index >= size(), in every build. */
				[[nodiscard]] constexpr auto operator[](const ca::usz index) const noexcept -> bool {

					this->_expect_index(index);
					return self::_test(this->_words(), index);
				}

				/* @DOC Bit at index, or nothing when index >= size(). */
				[[nodiscard]] constexpr auto get(const ca::usz index) const noexcept -> ca::optional<bool> {

					if (index >= this->_size())
						return ca::nullopt;
					return self::_test(this->_words(), index);
				}


				// -- modifiers ----------------------------------------------

				/* @DOC Sets the bit at index to value (1 by default). Traps when index >= size(), in
				   every build. */
				constexpr auto set(const ca::usz index, const bool value = true) noexcept -> void {

					this->_expect_index(index);
					ca::u64& word = this->_mutable_word(index);
					const ca::u64 mask = self::_mask(index);
					word = value ? word | mask : word & ~mask;
				}

				/* @DOC Clears the bit at index. Traps when index >= size(), in every build. */
				constexpr auto reset(const ca::usz index) noexcept -> void {
					this->set(index, false);
				}

				/* @DOC Inverts the bit at index. Traps when index >= size(), in every build. */
				constexpr auto flip(const ca::usz index) noexcept -> void {

					this->_expect_index(index);
					this->_mutable_word(index) ^= self::_mask(index);
				}

				constexpr auto set_all(void) noexcept -> void {

					for (ca::u64& word : this->_all_words())
						word = ~ca::u64{0U};
					this->_clear_tail();
				}

				constexpr auto reset_all(void) noexcept -> void {
					for (ca::u64& word : this->_all_words())
						word = 0U;
				}

				constexpr auto flip_all(void) noexcept -> void {

					for (ca::u64& word : this->_all_words())
						word = ~word;
					this->_clear_tail();
				}

				/* @DOC Bitwise and with other, of the same size. Traps when the sizes differ. */
				constexpr auto operator&=(const D& other) noexcept -> D& {

					const ca::borrowed_slice<ca::u64> words = this->_same_size(other);
					for (ca::usz i = 0U; i < words.size(); ++i)
						words.data()[i] &= other.words().data()[i];
					return static_cast<D&>(*this);
				}

				/* @DOC Bitwise or with other, of the same size. Traps when the sizes differ. */
				constexpr auto operator|=(const D& other) noexcept -> D& {

					const ca::borrowed_slice<ca::u64> words = this->_same_size(other);
					for (ca::usz i = 0U; i < words.size(); ++i)
						words.data()[i] |= other.words().data()[i];
					return static_cast<D&>(*this);
				}

				/* @DOC Bitwise exclusive or with other, of the same size. Traps when the sizes differ. */
				constexpr auto operator^=(const D& other) noexcept -> D& {

					const ca::borrowed_slice<ca::u64> words = this->_same_size(other);
					for (ca::usz i = 0U; i < words.size(); ++i)
						words.data()[i] ^= other.words().data()[i];
					return static_cast<D&>(*this);
				}


				// -- methods ------------------------------------------------

				/* @DOC Number of set bits. */
				[[nodiscard]] constexpr auto count(void) const noexcept -> ca::usz {

					ca::usz total = 0U;
					for (const ca::u64 word : this->_words())
						total += static_cast<ca::usz>(ca::popcount(word));
					return total;
				}

				/* @DOC Some bit is set. */
				[[nodiscard]] constexpr auto any(void) const noexcept -> bool {

					for (const ca::u64 word : this->_words()) {
						if (word != 0U)
							return true;
					}
					return false;
				}

				/* @DOC Every bit is set; true when empty. */
				[[nodiscard]] constexpr auto all(void) const noexcept -> bool {
					return !this->find_first_unset().has_value();
				}

				/* @DOC No bit is set. */
				[[nodiscard]] constexpr auto none(void) const noexcept -> bool {
					return !this->any();
				}

				/* @DOC Index of the first set bit, or nothing. */
				[[nodiscard]] constexpr auto find_first(void) const noexcept -> ca::optional<ca::usz> {
					return this->find_next(0U);
				}

				/* @DOC Index of the first set bit at or after from, or nothing. */
				[[nodiscard]] constexpr auto find_next(const ca::usz from) const noexcept -> ca::optional<ca::usz> {
					return self::_find(this->_words(), this->_size(), from, 0U);
				}

				/* @DOC Index of the first clear bit, or nothing. */
				[[nodiscard]] constexpr auto find_first_unset(void) const noexcept -> ca::optional<ca::usz> {
					return this->find_next_unset(0U);
				}

				/* @DOC Index of the first clear bit at or after from, or nothing. */
				[[nodiscard]] constexpr auto find_next_unset(const ca::usz from) const noexcept -> ca::optional<ca::usz> {
					return self::_find(this->_words(), this->_size(), from, ~ca::u64{0U});
				}

				/* @DOC The indices of the set bits, in increasing order. */
				[[nodiscard]] constexpr auto ones(void) const noexcept -> ca::impl::set_bits {

					const ca::borrowed_slice<const ca::u64> words = this->_words();
					return ca::impl::set_bits{words.data(), words.size()};
				}


			private:

				// -- private methods ----------------------------------------

				[[nodiscard]] constexpr auto _size(void) const noexcept -> ca::usz {
					return static_cast<const D&>(*this).size();
				}

				[[nodiscard]] constexpr auto _words(void) const noexcept -> ca::borrowed_slice<const ca::u64> {
					return static_cast<const D&>(*this).words();
				}

				[[nodiscard]] constexpr auto _all_words(void) noexcept -> ca::borrowed_slice<ca::u64> {
					return static_cast<D&>(*this)._mutable_words();
				}

				[[nodiscard]] constexpr auto _mutable_word(const ca::usz index) noexcept -> ca::u64& {
					return this->_all_words().data()[index / ca::impl::bit_word_size];
				}

				[[nodiscard]] static constexpr auto _mask(const ca::usz index) noexcept -> ca::u64 {
					return ca::u64{1U} << (index % ca::impl::bit_word_size);
				}

				[[nodiscard]] static constexpr auto _test(const ca::borrowed_slice<const ca::u64> words, const ca::usz index) noexcept -> bool {
					return (words.data()[index / ca::impl::bit_word_size] & self::_mask(index)) != 0U;
				}

				constexpr auto _expect_index(const ca::usz index) const noexcept -> void {
					if (index >= this->_size()) [[unlikely]]
						ca::impl::index_out_of_bounds();
				}

				[[nodiscard]] constexpr auto _same_size(const D& other) noexcept -> ca::borrowed_slice<ca::u64> {

					if (other.size() != this->_size()) [[unlikely]]
						ca::impl::index_out_of_bounds();
					return this->_all_words();
				}

				constexpr auto _clear_tail(void) noexcept -> void {

					const ca::usz used = this->_size() % ca::impl::bit_word_size;
					const ca::borrowed_slice<ca::u64> words = this->_all_words();
					if (used != 0U)
						words.data()[words.size() - 1U] &= (ca::u64{1U} << used) - 1U;
				}

				[[nodiscard]] static constexpr auto _find(const ca::borrowed_slice<const ca::u64> words, const ca::usz size, const ca::usz from, const ca::u64 invert) noexcept
				-> ca::optional<ca::usz> {

					if (from >= size)
						return ca::nullopt;
					ca::usz index = from / ca::impl::bit_word_size;
					ca::u64 word = (words.data()[index] ^ invert) & (~ca::u64{0U} << (from % ca::impl::bit_word_size));
					while (word == 0U) {
						if (++index == words.size())
							return ca::nullopt;
						word = words.data()[index] ^ invert;
					}
					const ca::usz found = index * ca::impl::bit_word_size + static_cast<ca::usz>(ca::countr_zero(word));
					if (found >= size)
						return ca::nullopt;
					return found;
				}

		}; // class bit_methods

	} // namespace impl

} // namespace ca

#endif // ca_collections_array_impl_bit_methods_hpp
