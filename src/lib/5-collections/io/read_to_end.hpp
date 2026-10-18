#ifndef ca_collections_io_read_to_end_hpp
#define ca_collections_io_read_to_end_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/io/is_buffered_reader.hpp"
#include "2-vocabulary/io/is_reader.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "5-collections/io/impl/is_char_buffer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- READ TO END ----------------------------------------------------

		/* @DOC Appends the rest of reader's input to out (ca::str, dynamic_array<char>,
		   inline_str<N>); how many bytes. A buffered reader's
		   chunks are copied straight from its buffer, others go through a 4 KiB stack chunk. On
		   failure out keeps what was read. */
		template <ca::io::is_reader R, ca::io::impl::is_char_buffer L>
		[[nodiscard]] auto read_to_end(R& reader, L& out) noexcept -> ca::result<ca::usz> {

			ca::usz total = 0U;
			for (;;) {
				if constexpr (ca::io::is_buffered_reader<R>) {
					auto chunk = reader.fill();
					if (!chunk) [[unlikely]]
						return chunk.propagate();
					if (chunk->empty())
						return total;
					auto appended = ca::io::impl::append_text(out, *chunk);
					if (!appended) [[unlikely]]
						return appended.propagate();
					total += chunk->size();
					reader.consume(chunk->size());
				}
				else {
					char chunk[4096];
					auto got = reader.read(chunk);
					if (!got) [[unlikely]]
						return got;
					if (*got == 0U)
						return total;
					auto appended = ca::io::impl::append_text(out, ca::sv::str{chunk, *got});
					if (!appended) [[unlikely]]
						return appended.propagate();
					total += *got;
				}
			}
		}

	} // namespace io

} // namespace ca

#endif // ca_collections_io_read_to_end_hpp
