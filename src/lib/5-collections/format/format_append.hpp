#ifndef ca_collections_format_format_append_hpp
#define ca_collections_format_format_append_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/type_identity.hpp"
#include "2-vocabulary/result.hpp"
#include "5-collections/format/format_string.hpp"
#include "5-collections/format/format_to.hpp"
#include "5-collections/format/is_displayable.hpp"
#include "5-collections/io/string_writer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FORMAT APPEND ------------------------------------------------------

	/* @DOC Formats args at the end of string (ca::str, any basic_str<char, A>,
	   inline_str<N>), `{}` syntax checked at compile time, in place. On failure (out of
	   memory, or errc::no_space for an inline_str) string is left as it was. */
	template <ca::io::impl::is_char_string S, typename... Args>
	requires (ca::is_displayable<Args> && ...)
	[[nodiscard]] auto format_append(S& string, const ca::format_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> ca::result<void> {

		const ca::usz size = string.size();
		ca::io::string_writer<S> writer{string};
		auto written = ca::format_to(writer, format, args...);
		if (!written) [[unlikely]] {
			string.set_size(size);
			return written.propagate();
		}
		return ca::result<void>{};
	}

} // namespace ca

#endif // ca_collections_format_format_append_hpp
