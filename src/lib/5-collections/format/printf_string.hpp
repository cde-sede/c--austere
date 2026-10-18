#ifndef ca_collections_format_printf_string_hpp
#define ca_collections_format_printf_string_hpp

#include "1-primitives/integers/usz.hpp"
#include "5-collections/format/impl/display_kind.hpp"
#include "5-collections/format/impl/format_piece.hpp"
#include "5-collections/format/impl/format_spec.hpp"
#include "5-collections/format/impl/parse_printf.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- PRINTF STRING ------------------------------------------------------

	/* @DOC A printf format for arguments Args, parsed and checked when it compiles: each
	   conversion must fit its argument's type. `%[flags][width][.precision][length]conversion`:
	   - flags `-` `+` space `#` `0`; length modifiers (hh h l ll j z t L q) are read and
	     ignored, the type is known
	   - d i u x X o b B c p f F e E g G a A, `%%`; u x X o b B show a negative signed value as
	     the unsigned of its width, as C does
	   - `%s` prints any displayable argument its default way (strings, bools, errors, paths,
	     custom types); `%ls` `%lc` are `%s` `%c`, wide text transcoded to UTF-8
	   - width and a string's precision count code points, not bytes
	   - no `*` width or precision, no `%n`
	   Shares the engine of format_string; output matches C's except `%#x` of 0 ("0x0"). */
	template <typename... Args>
	class printf_string final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::printf_string<Args...>;


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

			consteval printf_string(const char* const text) noexcept
			: _text{text}, _pieces{}, _specs{} {
				ca::impl::parse_printf(text, _kinds, COUNT, _pieces, _specs);
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

	}; // class printf_string

} // namespace ca

#endif // ca_collections_format_printf_string_hpp
