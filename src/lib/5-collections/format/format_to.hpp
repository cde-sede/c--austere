/* @DOC Formatting into writers: format_to ({}) and printf_to (%), the engine under ca::print.

   Performance:
   - Formats are parsed and checked at compile time into pieces and specs; nothing is parsed
     at run time. Arguments are type-erased into 24-byte records, so one out-of-line engine
     serves every call site: small code per call, no varargs.
   - Text is written in place into the writer's window (memory_writer, buffered
     ca::io::writer); writers without one are staged on a 1 KiB stack buffer.
   - Width and precision count code points: ASCII text is checked with a few word loads,
     anything else decoded. Wide text is transcoded through a 256-byte stack buffer.
   - Integers: two digits per division (decimal), shifts for x/o/b. Floats go through
     ca::sys::format_float (snprintf); the shortest form takes up to three snprintf/strtod
     rounds.

   Baseline snprintf, and fprintf for the buffered line:
   --BENCH bench/format.cpp
   bench/format.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | 3 integers, printf_to, 1e6 | 0.59x | 0.57x | std: snprintf |
   | 3 integers, format_to, 1e6 | 0.59x | 0.57x | std: snprintf with the % spelling |
   | %-8s %6d %08x, 1e6 | 0.67x | 0.62x | |
   | 3 strings, 1e6 | 0.71x | 0.68x | |
   | %.3f, 1e6 | 1.10x | 1.15x | ca formats fixed floats through snprintf |
   | shortest float {}, 1e6 | 1.38x | 1.45x | std: %.17g, which is not the shortest round trip |
   | buffered fd line, ca::printf, 1e6 | 0.63x | 0.60x | std: fprintf to /dev/null |
   BENCH-- */
#ifndef ca_collections_format_format_to_hpp
#define ca_collections_format_format_to_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/traits/type_identity.hpp"
#include "2-vocabulary/io/is_writer.hpp"
#include "2-vocabulary/result.hpp"
#include "5-collections/format/format_string.hpp"
#include "5-collections/format/impl/display_argument.hpp"
#include "5-collections/format/impl/emit.hpp"
#include "5-collections/format/impl/render.hpp"
#include "5-collections/format/is_displayable.hpp"
#include "5-collections/format/text_writer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FORMAT TO ----------------------------------------------------------

	/* @DOC Formats args into a writer (ca::io::is_writer), `{}` syntax (see format_string),
	   checked at compile time: `ca::format_to(m, "{} items", n)`. The message reaches the writer
	   whole, in place when it lends a window; returns its size in bytes, or the writer's error
	   for it (also kept sticky by the writer). With a text_writer: nested formatting inside a
	   display member, returning nothing: a failure stays in the text_writer, for the outer
	   call. */
	template <typename S, typename... Args>
	requires (ca::io::is_writer<ca::remove_cvref<S>> && (ca::is_displayable<Args> && ...))
	[[nodiscard]] auto format_to(S&& writer, const ca::format_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> ca::result<ca::usz> {
		if constexpr (sizeof...(Args) == 0U)
			return ca::impl::emit(writer, format.text(), format.pieces(), format.specs(), nullptr, 0U);
		else {
			const ca::impl::display_argument arguments[] = {ca::impl::make_display_argument(args)...};
			return ca::impl::emit(writer, format.text(), format.pieces(), format.specs(), arguments, sizeof...(Args));
		}
	}

	template <typename... Args>
	requires (ca::is_displayable<Args> && ...)
	auto format_to(ca::text_writer& out, const ca::format_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> void {
		if constexpr (sizeof...(Args) == 0U)
			ca::impl::render(out, format.text(), format.pieces(), format.specs(), nullptr, 0U);
		else {
			const ca::impl::display_argument arguments[] = {ca::impl::make_display_argument(args)...};
			ca::impl::render(out, format.text(), format.pieces(), format.specs(), arguments, sizeof...(Args));
		}
	}

} // namespace ca

#endif // ca_collections_format_format_to_hpp
