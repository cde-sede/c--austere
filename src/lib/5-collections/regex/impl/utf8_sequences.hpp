#ifndef ca_collections_regex_impl_utf8_sequences_hpp
#define ca_collections_regex_impl_utf8_sequences_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UTF8 SEQUENCES -----------------------------------------------------

	namespace impl {

		/* @DOC One to four byte ranges: the UTF-8 encodings of a run of code points, byte i in
		   low[i]..high[i]. */
		struct utf8_sequence final {

				using self = ca::impl::utf8_sequence;

				ca::u8 _low[4];
				ca::u8 _high[4];
				ca::u8 _length;

		}; // struct utf8_sequence

		/* @DOC The code points first..last (surrogates skipped) as UTF-8 byte range sequences, in
		   increasing order, each a product of ranges: what an automaton over bytes matches a
		   range of code points with. At most 7 sequences per encoding length. */
		class utf8_sequences final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::utf8_sequences;

				static constexpr ca::u32 _encoding_limits[3] = {0x7FU, 0x7FFU, 0xFFFFU};


			private:

				// -- private members ----------------------------------------

				ca::u32 _first[16];
				ca::u32 _last[16];
				ca::usz _count;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr utf8_sequences(const char32_t first, const char32_t last) noexcept
				: _first{static_cast<ca::u32>(first)}, _last{static_cast<ca::u32>(last)}, _count{1U} {
				}


				// -- methods ------------------------------------------------

				/* @DOC The next sequence into out; false when none is left. */
				[[nodiscard]] constexpr auto next(ca::impl::utf8_sequence& out) noexcept -> bool {

					while (_count != 0U) {
						--_count;
						ca::u32 first = _first[_count];
						ca::u32 last = _last[_count];
						if (self::_narrow(first, last))
							return self::_encode(first, last, out);
					}
					return false;
				}


			private:

				// -- private methods ----------------------------------------

				constexpr auto _push(const ca::u32 first, const ca::u32 last) noexcept -> void {

					_first[_count] = first;
					 _last[_count] = last;
					++_count;
				}

				[[nodiscard]] constexpr auto _narrow(ca::u32& first, ca::u32& last) noexcept -> bool {
					while (true) {
						if (first < 0xE000U && last > 0xD7FFU) {
							this->_push(0xE000U, last);
							last = 0xD7FFU;
							if (first > last)
								return false;
							continue;
						}
						if (first > last)
							return false;
						bool split = false;
						for (const ca::u32 max : _encoding_limits) {
							if (first <= max && max < last) {
								this->_push(max + 1U, last);
								 last = max;
								split = true;
								break;
							}
						}
						if (split)
							continue;
						if (last <= 0x7FU)
							return true;
						for (ca::u32 i = 1U; i < 4U && !split; ++i) {
							const ca::u32 mask = (1U << (6U * i)) - 1U;
							if ((first & ~mask) == (last & ~mask))
								continue;
							if ((first & mask) != 0U) {
								this->_push((first | mask) + 1U, last);
								 last = first | mask;
								split = true;
							}
							else if ((last & mask) != mask) {
								this->_push(last & ~mask, last);
								 last = (last & ~mask) - 1U;
								split = true;
							}
						}
						if (!split)
							return true;
					}
				}

				[[nodiscard]] static constexpr auto _encode_point(const ca::u32 point, ca::u8* const out) noexcept -> ca::u8 {

					if (point < 0x80U) {
						out[0] = static_cast<ca::u8>(point);
						return 1U;
					}
					if (point < 0x800U) {
						out[0] = static_cast<ca::u8>(0xC0U | (point >> 6U));
						out[1] = static_cast<ca::u8>(0x80U | (point & 0x3FU));
						return 2U;
					}
					if (point < 0x10000U) {
						out[0] = static_cast<ca::u8>(0xE0U | (point >> 12U));
						out[1] = static_cast<ca::u8>(0x80U | ((point >> 6U) & 0x3FU));
						out[2] = static_cast<ca::u8>(0x80U | (point & 0x3FU));
						return 3U;
					}
					out[0] = static_cast<ca::u8>(0xF0U | (point >> 18U));
					out[1] = static_cast<ca::u8>(0x80U | ((point >> 12U) & 0x3FU));
					out[2] = static_cast<ca::u8>(0x80U | ((point >> 6U) & 0x3FU));
					out[3] = static_cast<ca::u8>(0x80U | (point & 0x3FU));
					return 4U;
				}

				[[nodiscard]] static constexpr auto _encode(const ca::u32 first, const ca::u32 last, ca::impl::utf8_sequence& out) noexcept -> bool {

					out._length = self::_encode_point(first, out._low);
					static_cast<void>(self::_encode_point(last, out._high));
					return true;
				}

		}; // class utf8_sequences

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_utf8_sequences_hpp
