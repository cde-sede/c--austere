/* @DOC Buffered input and output over file descriptors: sockets, terminals, pipe pairs.

   Performance:
   - Layout: a ca::io::writer (40 bytes) and a ca::io::reader (48), 88 bytes on 64-bit. Both
     buffers are borrowed, never allocated.
   - Every call forwards inline to the reader or the writer: same in-place fill/read_until and
     window/drain/settle, same costs. The tie adds one null check before each read that may
     block. */
#ifndef ca_resources_io_stream_hpp
#define ca_resources_io_stream_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "6-resources/io/buffering.hpp"
#include "6-resources/io/reader.hpp"
#include "6-resources/io/writer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- STREAM ---------------------------------------------------------

		/* @DOC Stream (ca::io::is_stream) over descriptors it does not own: a ca::io::reader on
		   the input one and a ca::io::writer on the output one (the same for a socket or a
		   terminal), each buffering into memory the caller provides. The reader is tied to the
		   writer: pending output is written before a read may block, so a request is sent before
		   its answer is awaited. Reads report errors per call, writes per call and sticky (see ca::io::writer). Cleanup
		   is flush(); the destructor is a failsafe writing out pending bytes silently. Not
		   thread-safe. */
		class stream final {


			private:

				// -- private types ------------------------------------------

				using self = ca::io::stream;


				// -- private members ----------------------------------------

				ca::io::writer _writer;
				ca::io::reader _reader;


			public:

				// -- public lifecycle ---------------------------------------

				/* @DOC Closed: every read and write fails with errc::bad_descriptor. */
				constexpr stream(void) noexcept
				: _writer{}, _reader{} {
				}

				/* @DOC Reads input, writes output (may equal input), with buffers (char arrays, or
				   ca::span{data, capacity}) that must outlive the stream; an empty one makes that side
				   unbuffered. */
				constexpr stream(const int input, const int output, const ca::io::buffering mode, const ca::span<char> input_buffer, const ca::span<char> output_buffer) noexcept
				: _writer{output, mode, output_buffer}, _reader{input, input_buffer} {
					_reader.tie(&_writer);
				}

				/* @DOC Over one descriptor read and written both ways (socket, terminal). */
				constexpr stream(const int fd, const ca::io::buffering mode, const ca::span<char> input_buffer, const ca::span<char> output_buffer) noexcept
				: self{fd, fd, mode, input_buffer, output_buffer} {
				}

				/* @DOC Over file's descriptor (a ca::fs::descriptor, or anything with native() -> int),
				   borrowed: file must stay open while the stream is used. */
				template <ca::io::impl::has_native_descriptor D>
				constexpr stream(const D& file, const ca::io::buffering mode, const ca::span<char> input_buffer, const ca::span<char> output_buffer) noexcept
				: self{file.native(), file.native(), mode, input_buffer, output_buffer} {
				}

				stream(const self&) = delete;

				constexpr stream(self&& other) noexcept
				: _writer{ca::move(other._writer)}, _reader{ca::move(other._reader)} {
					if (_reader.tied() != nullptr)
						_reader.tie(&_writer);
				}


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;

				auto operator=(self&&) -> self& = delete;


				// -- accessors ----------------------------------------------

				/* @DOC The input side, for code taking a ca::io::reader& (or to untie it). */
				[[nodiscard]] constexpr auto reader(void) noexcept -> ca::io::reader& {
					return _reader;
				}

				/* @DOC The output side, for code taking a ca::io::writer&. */
				[[nodiscard]] constexpr auto writer(void) noexcept -> ca::io::writer& {
					return _writer;
				}

				[[nodiscard]] constexpr auto input(void) const noexcept -> int {
					return _reader.native();
				}

				[[nodiscard]] constexpr auto output(void) const noexcept -> int {
					return _writer.native();
				}

				/* @DOC Bytes buffered and not consumed yet. */
				[[nodiscard]] constexpr auto available(void) const noexcept -> ca::usz {
					return _reader.available();
				}

				/* @DOC Bytes buffered, not yet written out. */
				[[nodiscard]] constexpr auto pending(void) const noexcept -> ca::usz {
					return _writer.pending();
				}

				/* @DOC The writer's sticky error, ok when none; kept until flush() or clear_error(). A
				   tied reader's write-out failures land here. */
				[[nodiscard]] constexpr auto error(void) const noexcept -> ca::error {
					return _writer.error();
				}


				// -- methods ------------------------------------------------

				/* @DOC See ca::io::reader::read. */
				[[nodiscard]] auto read(const ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz> {
					return _reader.read(buffer);
				}

				[[nodiscard]] auto read_full(const ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz> {
					return _reader.read_full(buffer);
				}

				[[nodiscard]] auto fill(void) noexcept -> ca::result<ca::sv::str> {
					return _reader.fill();
				}

				constexpr auto consume(const ca::usz count) noexcept -> void {
					_reader.consume(count);
				}

				[[nodiscard]] auto read_until(const char delimiter) noexcept -> ca::result<ca::sv::str> {
					return _reader.read_until(delimiter);
				}

				/* @DOC See ca::io::writer::write. */
				[[nodiscard]] auto write(const ca::span<const ca::byte> data) noexcept -> ca::result<void> {
					return _writer.write(data);
				}

				[[nodiscard]] auto write_pending(void) noexcept -> ca::result<void> {
					return _writer.write_pending();
				}

				[[nodiscard]] auto flush(void) noexcept -> ca::result<void> {
					return _writer.flush();
				}

				constexpr auto clear_error(void) noexcept -> void {
					_writer.clear_error();
				}

				constexpr auto discard(void) noexcept -> void {
					_writer.discard();
				}

				[[nodiscard]] constexpr auto window(void) const noexcept -> ca::span<char> {
					return _writer.window();
				}

				[[nodiscard]] auto drain(char* const at) noexcept -> ca::result<ca::span<char>> {
					return _writer.drain(at);
				}

				[[nodiscard]] auto settle(char* const at) noexcept -> ca::result<void> {
					return _writer.settle(at);
				}

		}; // class stream

	} // namespace io

} // namespace ca

#endif // ca_resources_io_stream_hpp
