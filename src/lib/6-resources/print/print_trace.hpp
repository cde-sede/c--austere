#ifndef ca_resources_print_print_trace_hpp
#define ca_resources_print_print_trace_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/io/is_writer.hpp"
#include "3-system/trace/of.hpp"
#include "5-collections/format/format_to.hpp"
#include "6-resources/io/err.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- PRINT TRACE --------------------------------------------------------

	/* @DOC Prints error and its thread trace (see ca::trace::record), then a newline:
	   `ca::print_trace(r.error())` to ca::io::err(), `ca::print_trace(writer, e)` to any writer.
	   An error without the current trace prints its message only. A ca::report prints with
	   ca::print and `{}`. Errors as print. */
	[[nodiscard]] inline auto print_trace(const ca::error error) noexcept -> ca::result<ca::usz> {
		return ca::format_to(ca::io::err(), "{}\n", ca::trace::of(error));
	}

	template <typename S>
	requires ca::io::is_writer<ca::remove_cvref<S>>
	[[nodiscard]] auto print_trace(S&& writer, const ca::error error) noexcept -> ca::result<ca::usz> {
		return ca::format_to(writer, "{}\n", ca::trace::of(error));
	}

} // namespace ca

#endif // ca_resources_print_print_trace_hpp
