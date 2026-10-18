#ifndef ca_collections_format_format_string_hpp
#define ca_collections_format_format_string_hpp

#include "1-primitives/integers/usz.hpp"
#include "5-collections/format/impl/display_kind.hpp"
#include "5-collections/format/impl/format_piece.hpp"
#include "5-collections/format/impl/format_spec.hpp"
#include "5-collections/format/impl/parse_braces.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FORMAT STRING ------------------------------------------------------

	/* @DOC A print format for arguments Args, parsed and checked when it compiles: a mismatch is
	   a compile error naming it. Placeholders `{}` or `{spec}` (a leading ':' is allowed), spec
	   being `[[fill]align][sign][#][0][width][.precision][type]`:
	   - align `<` `>` `^`; default: numbers right, text left; fill a char, default space
	   - sign `+` or space; `#` adds 0x 0X 0b 0B 0 prefixes; `0` pads numbers with zeros
	   - width and a string's precision count code points, not bytes
	   - precision: float digits, max code points of a string, min digits of an integer
	   - type: d x X o b B c (integers, chars, bools; d also an error's code), s (text),
	     p (pointers), f F e E g G a A (floats), ? (strings and characters quoted and escaped:
	     \t \n \r \0 \\ the quote, other controls \u{..}, invalid UTF-8 bytes \x{..})
	   - `{{` and `}}` print one brace
	   Text of any code unit prints as UTF-8: char and char8_t as is; char16_t, char32_t and
	   wchar_t characters, pointers, arrays, views and strings transcoded, invalid sequences as
	   U+FFFD. Floats print shortest round-trip by default. Custom types (display member) take
	   `{}` only. */
	template <typename... Args>
	class format_string final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::format_string<Args...>;


		public:

			// -- public constants -------------------------------------------

			static constexpr ca::usz COUNT = sizeof...(Args);


		private:

			// -- private members --------------------------------------------

			static constexpr ca::impl::display_kind _kinds[COUNT + 1U] = {ca::impl::display_kind_of<Args>..., ca::impl::display_kind::none};

			const char* _text;
			ca::impl::format_piece _pieces[COUNT + 1U];
			ca::impl::format_spec _specs[COUNT + 1U];


		public:

			// -- public lifecycle -------------------------------------------

			consteval format_string(const char* const text) noexcept
			: _text{text}, _pieces{}, _specs{} {
				ca::impl::parse_braces(text, _kinds, COUNT, _pieces, _specs);
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto text(void) const noexcept -> const char* {
				return _text;
			}

			[[nodiscard]] constexpr auto pieces(void) const noexcept -> const ca::impl::format_piece* {
				return _pieces;
			}

			[[nodiscard]] constexpr auto specs(void) const noexcept -> const ca::impl::format_spec* {
				return _specs;
			}

	}; // class format_string

} // namespace ca

#endif // ca_collections_format_format_string_hpp
