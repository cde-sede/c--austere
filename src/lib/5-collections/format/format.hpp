#ifndef ca_collections_format_format_hpp
#define ca_collections_format_format_hpp

#include "1-primitives/traits/type_identity.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "5-collections/format/format_string.hpp"
#include "5-collections/format/format_to.hpp"
#include "5-collections/format/is_displayable.hpp"
#include "5-collections/io/string_writer.hpp"
#include "5-collections/string/basic_str.hpp"
#include "5-collections/string/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FORMAT -------------------------------------------------------------

	/* @DOC Formats args into a new string, `{}` syntax checked at compile time (see
	   format_string): `auto s = ca::format("{}-{:04}", name, id);`. Formatted in place in the
	   string, which grows geometrically; short results stay inline. With an allocator first,
	   the string draws from it. Fails only when memory runs out. */
	template <typename... Args>
	requires (ca::is_displayable<Args> && ...)
	[[nodiscard]] auto format(const ca::format_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> ca::result<ca::str> {

		ca::str out{};
		ca::io::string_writer<ca::str> writer{out};
		auto written = ca::format_to(writer, format, args...);
		if (!written) [[unlikely]]
			return written.propagate();
		return out;
	}

	template <ca::is_allocator A, typename... Args>
	requires (ca::is_displayable<Args> && ...)
	[[nodiscard]] auto format(const A& allocator, const ca::format_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> ca::result<ca::basic_str<char, A>> {

		ca::basic_str<char, A> out{allocator};
		ca::io::string_writer<ca::basic_str<char, A>> writer{out};
		auto written = ca::format_to(writer, format, args...);
		if (!written) [[unlikely]]
			return written.propagate();
		return out;
	}

} // namespace ca

#endif // ca_collections_format_format_hpp
