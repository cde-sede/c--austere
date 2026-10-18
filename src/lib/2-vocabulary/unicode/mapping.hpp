#ifndef ca_vocabulary_unicode_mapping_hpp
#define ca_vocabulary_unicode_mapping_hpp

#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- MAPPING --------------------------------------------------------

		/* @DOC What a full case mapping turns one code point into: 1 to 3 code points (sharp s, U+00DF,
		   uppercases to "SS"). Iterable. */
		class mapping final {


			private:

				// -- private types ------------------------------------------

				using self = ca::unicode::mapping;


				// -- private members ----------------------------------------

				char32_t _points[3];
				ca::u8 _size;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr explicit mapping(const char32_t point) noexcept
				: _points{point, U'\0', U'\0'}, _size{1U} {
				}

				/* @DOC The non-zero prefix of points, at least one. */
				constexpr explicit mapping(const char32_t (&points)[3]) noexcept
				: _points{points[0], points[1], points[2]}, _size{static_cast<ca::u8>(points[1] == U'\0' ? 1U : points[2] == U'\0' ? 2U : 3U)} {
				}


				// -- iterators ----------------------------------------------

				[[nodiscard]] constexpr auto begin(void) const noexcept -> const char32_t* {
					return _points;
				}

				[[nodiscard]] constexpr auto end(void) const noexcept -> const char32_t* {
					return _points + _size;
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
					return _size;
				}

				/* @DOC The first code point: the whole mapping when size() is 1. */
				[[nodiscard]] constexpr auto first(void) const noexcept -> char32_t {
					return _points[0];
				}

		}; // class mapping

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_mapping_hpp
