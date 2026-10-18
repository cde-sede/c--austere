#ifndef ca_vocabulary_unicode_decoded_hpp
#define ca_vocabulary_unicode_decoded_hpp

#include "1-primitives/integers/u8.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- DECODED --------------------------------------------------------

		/* @DOC One code point read from encoded text: the value (U+FFFD when the sequence is
		   invalid), how many code units it took (1 to 4; an invalid sequence takes its maximal
		   valid prefix, at least 1, as Unicode recommends), and whether it was valid. */
		class decoded final {


			private:

				// -- private types ------------------------------------------

				using self = ca::unicode::decoded;


				// -- private members ----------------------------------------

				char32_t _code_point;
				ca::u8 _length;
				bool _valid;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr decoded(const char32_t code_point, const ca::u8 length, const bool valid) noexcept
				: _code_point{code_point}, _length{length}, _valid{valid} {
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto code_point(void) const noexcept -> char32_t {
					return _code_point;
				}

				/* @DOC Code units consumed. */
				[[nodiscard]] constexpr auto length(void) const noexcept -> ca::u8 {
					return _length;
				}

				[[nodiscard]] constexpr auto valid(void) const noexcept -> bool {
					return _valid;
				}

		}; // class decoded

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_decoded_hpp
