#ifndef ca_collections_io_impl_is_char_buffer_hpp
#define ca_collections_io_impl_is_char_buffer_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- IS CHAR BUFFER -------------------------------------------------

		namespace impl {

			/* @DOC Where read_line and read_to_end put text: dynamic_array<char, A> (extend),
			   basic_str<char, A>, inline_str<N> (append). */
			template <typename L>
			concept is_char_buffer = ca::is_same<typename L::value_type, char> && requires (L& line, const ca::usz size) {
				{ line.resize(size) } -> ca::is_same<ca::result<void>>;
			} && (requires (L& line, const ca::sv::str text) {
				{ line.extend(text) } -> ca::is_same<ca::result<void>>;
			} || requires (L& line, const ca::sv::str text) {
				{ line.append(text) } -> ca::is_same<ca::result<void>>;
			});

			/* @DOC text added to the end of buffer, by extend or append. */
			template <ca::io::impl::is_char_buffer L>
			[[nodiscard]] auto append_text(L& buffer, const ca::sv::str text) noexcept -> ca::result<void> {

				if constexpr (requires { buffer.extend(text); })
					return buffer.extend(text);
				else
					return buffer.append(text);
			}

		} // namespace impl

	} // namespace io

} // namespace ca

#endif // ca_collections_io_impl_is_char_buffer_hpp
