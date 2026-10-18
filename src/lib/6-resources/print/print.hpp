#ifndef ca_resources_print_print_hpp
#define ca_resources_print_print_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/traits/type_identity.hpp"
#include "2-vocabulary/io/is_writer.hpp"
#include "5-collections/format/format_string.hpp"
#include "5-collections/format/format_to.hpp"
#include "5-collections/format/is_displayable.hpp"
#include "6-resources/io/out.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- PRINT --------------------------------------------------------------

	/* @DOC Prints args, `{}` syntax (see format_string), checked at compile time:
	   `ca::print("{} items in {}\n", n, path)` goes to ca::io::out(), this thread's buffered
	   stdout; `ca::print(writer, ...)` to any writer: ca::io::err() or
	   ca::io::writer::unbuffered(fd) write at once. Each message reaches the writer whole.
	   Returns the message's size in bytes (buffered: once the writer holds it), or its error;
	   the writer also keeps the error sticky, for a final check with its flush() (ca::flush()
	   for stdout), clear_error() once handled. */
	template <typename... Args>
	requires (ca::is_displayable<Args> && ...)
	[[nodiscard]] auto print(const ca::format_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> ca::result<ca::usz> {
		return ca::format_to(ca::io::out(), format, args...);
	}

	template <typename S, typename... Args>
	requires (ca::io::is_writer<ca::remove_cvref<S>> && (ca::is_displayable<Args> && ...))
	[[nodiscard]] auto print(S&& writer, const ca::format_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> ca::result<ca::usz> {
		return ca::format_to(writer, format, args...);
	}

} // namespace ca

#endif // ca_resources_print_print_hpp
