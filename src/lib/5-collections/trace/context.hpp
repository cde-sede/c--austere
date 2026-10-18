#ifndef ca_collections_trace_context_hpp
#define ca_collections_trace_context_hpp

#include "1-primitives/traits/type_identity.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "5-collections/format/is_displayable.hpp"
#include "5-collections/format/located_format_string.hpp"
#include "5-collections/trace/report.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- CONTEXT ------------------------------------------------------------

	/* @DOC Adds a formatted frame here (the caller) and returns the report to return, as E of a
	   result<T, ca::report<A>>. From an error, the report starts with a copy of its thread
	   trace: `if (!f) return ca::context(f.error(), "opening {}", path);`. From a report, it
	   grows it: `return ca::context(ca::move(r).error(), "loading {}", name);`. */
	template <typename... Args>
	requires (ca::is_displayable<Args> && ...)
	[[nodiscard, gnu::cold]] auto context(const ca::error error, const ca::located_format_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> ca::failure<ca::report<>> {

		auto report = ca::report<>::of(error);
		report.context(format, args...);
		return ca::failure{ca::move(report)};
	}

	template <ca::is_allocator A, typename... Args>
	requires (ca::is_displayable<Args> && ...)
	[[nodiscard, gnu::cold]] auto context(ca::report<A>&& report, const ca::located_format_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> ca::failure<ca::report<A>> {

		report.context(format, args...);
		return ca::failure{ca::move(report)};
	}

} // namespace ca

#endif // ca_collections_trace_context_hpp
