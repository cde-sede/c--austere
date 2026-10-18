#ifndef ca_collections_format_printf_to_hpp
#define ca_collections_format_printf_to_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/traits/type_identity.hpp"
#include "2-vocabulary/io/is_writer.hpp"
#include "2-vocabulary/result.hpp"
#include "5-collections/format/printf_string.hpp"
#include "5-collections/format/impl/display_argument.hpp"
#include "5-collections/format/impl/emit.hpp"
#include "5-collections/format/impl/render.hpp"
#include "5-collections/format/is_displayable.hpp"
#include "5-collections/format/text_writer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- PRINTF TO ----------------------------------------------------------

	/* @DOC Formats args into a writer (ca::io::is_writer), C % syntax (see printf_string),
	   checked at compile time: `ca::printf_to(m, "%d items", n)`. The message reaches the writer
	   whole, in place when it lends a window; returns its size in bytes, or the writer's error
	   for it (also kept sticky by the writer). With a text_writer: nested formatting inside a
	   display member, returning nothing: a failure stays in the text_writer, for the outer
	   call. */
	template <typename S, typename... Args>
	requires (ca::io::is_writer<ca::remove_cvref<S>> && (ca::is_displayable<Args> && ...))
	[[nodiscard]] auto printf_to(S&& writer, const ca::printf_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> ca::result<ca::usz> {
		if constexpr (sizeof...(Args) == 0U)
			return ca::impl::emit(writer, format.text(), format.pieces(), format.specs(), nullptr, 0U);
		else {
			const ca::impl::display_argument arguments[] = {ca::impl::make_display_argument(args)...};
			return ca::impl::emit(writer, format.text(), format.pieces(), format.specs(), arguments, sizeof...(Args));
		}
	}

	template <typename... Args>
	requires (ca::is_displayable<Args> && ...)
	auto printf_to(ca::text_writer& out, const ca::printf_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> void {
		if constexpr (sizeof...(Args) == 0U)
			ca::impl::render(out, format.text(), format.pieces(), format.specs(), nullptr, 0U);
		else {
			const ca::impl::display_argument arguments[] = {ca::impl::make_display_argument(args)...};
			ca::impl::render(out, format.text(), format.pieces(), format.specs(), arguments, sizeof...(Args));
		}
	}

} // namespace ca

#endif // ca_collections_format_printf_to_hpp
