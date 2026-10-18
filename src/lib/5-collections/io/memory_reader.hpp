#ifndef ca_collections_io_memory_reader_hpp
#define ca_collections_io_memory_reader_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/find_byte.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- MEMORY READER --------------------------------------------------

		/* @DOC Buffered reader over text it does not own, which must outlive it: parse a string
		   with the code that parses a file. Never fails; the end of the text is the end of
		   input. */
		class memory_reader final {


			private:

				// -- private types ------------------------------------------

				using self = ca::io::memory_reader;


				// -- private members ----------------------------------------

				const char* _at;
				const char* _end;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr memory_reader(const ca::sv::str text) noexcept
				: _at{text.data()}, _end{text.data() + text.size()} {
				}


				// -- accessors ----------------------------------------------

				/* @DOC The text not read yet. */
				[[nodiscard]] constexpr auto remaining(void) const noexcept -> ca::sv::str {
					return ca::sv::str{_at, static_cast<ca::usz>(_end - _at)};
				}

				[[nodiscard]] constexpr auto at_end(void) const noexcept -> bool {
					return _at == _end;
				}


				// -- methods ------------------------------------------------

				[[nodiscard]] auto read(const ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz> {

					const ca::usz left = static_cast<ca::usz>(_end - _at);
					const ca::usz count = buffer.size() < left ? buffer.size() : left;
					ca::copy_bytes(buffer.data(), _at, count);
					_at += count;
					return count;
				}

				/* @DOC read until buffer is full or the end; how many. */
				[[nodiscard]] auto read_full(const ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz> {
					return this->read(buffer);
				}

				/* @DOC All the text not read yet. */
				[[nodiscard]] constexpr auto fill(void) const noexcept -> ca::result<ca::sv::str> {
					return this->remaining();
				}

				constexpr auto consume(const ca::usz count) noexcept -> void {
					_at += count < static_cast<ca::usz>(_end - _at) ? count : static_cast<ca::usz>(_end - _at);
				}

				/* @DOC The text up to and including the next delimiter, or the rest; empty at the end. */
				[[nodiscard]] auto read_until(const char delimiter) noexcept -> ca::result<ca::sv::str> {

					const char* const found = ca::find_byte(_at, delimiter, static_cast<ca::usz>(_end - _at));
					const char* const stop = found != nullptr ? found + 1 : _end;
					const ca::sv::str part{_at, static_cast<ca::usz>(stop - _at)};
					_at = stop;
					return part;
				}

		}; // class memory_reader

	} // namespace io

} // namespace ca

#endif // ca_collections_io_memory_reader_hpp
