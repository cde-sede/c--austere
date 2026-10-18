#ifndef ca_resources_print_printf_hpp
#define ca_resources_print_printf_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/traits/type_identity.hpp"
#include "2-vocabulary/io/is_writer.hpp"
#include "5-collections/format/printf_string.hpp"
#include "5-collections/format/printf_to.hpp"
#include "5-collections/format/is_displayable.hpp"
#include "6-resources/io/out.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- PRINTF -------------------------------------------------------------

	/* @DOC print with C % syntax (see printf_string), typed and checked at compile time, no
	   varargs: `ca::printf("%-8s|%5.2f\n", name, ratio)`. Same targets, buffering and errors
	   (per call and sticky) as print. */
	template <typename... Args>
	requires (ca::is_displayable<Args> && ...)
	[[nodiscard]] auto printf(const ca::printf_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> ca::result<ca::usz> {
		return ca::printf_to(ca::io::out(), format, args...);
	}

	template <typename S, typename... Args>
	requires (ca::io::is_writer<ca::remove_cvref<S>> && (ca::is_displayable<Args> && ...))
	[[nodiscard]] auto printf(S&& writer, const ca::printf_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> ca::result<ca::usz> {
		return ca::printf_to(writer, format, args...);
	}

} // namespace ca

#endif // ca_resources_print_printf_hpp
