#ifndef ca_collections_io_read_line_hpp
#define ca_collections_io_read_line_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/find_byte.hpp"
#include "2-vocabulary/io/is_buffered_reader.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "5-collections/io/impl/is_char_buffer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- READ LINE ------------------------------------------------------

		/* @DOC Replaces line (ca::str, dynamic_array<char>, inline_str<N>: errc::no_space
		   past N) with the next line of reader, '\n' excluded ("\r" kept), whatever its length;
		   false at end of input when nothing was left. Chunks are copied from the
		   reader's buffer straight into line. On failure line holds what was read. For a line
		   that fits the buffer, read_until('\n') gives it without a copy. */
		template <ca::io::is_buffered_reader R, ca::io::impl::is_char_buffer L>
		[[nodiscard]] auto read_line(R& reader, L& line) noexcept -> ca::result<bool> {

			auto cleared = line.resize(0U);
			if (!cleared) [[unlikely]]
				return cleared.propagate();
			bool any = false;
			for (;;) {
				auto chunk = reader.fill();
				if (!chunk) [[unlikely]]
					return chunk.propagate();
				if (chunk->empty())
					return any;
				any = true;
				const char* const end = ca::find_byte(chunk->data(), '\n', chunk->size());
				const ca::usz kept = end != nullptr ? static_cast<ca::usz>(end - chunk->data()) : chunk->size();
				auto appended = ca::io::impl::append_text(line, chunk->prefix(kept));
				if (!appended) [[unlikely]]
					return appended.propagate();
				reader.consume(end != nullptr ? kept + 1U : kept);
				if (end != nullptr)
					return true;
			}
		}

	} // namespace io

} // namespace ca

#endif // ca_collections_io_read_line_hpp
