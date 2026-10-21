/* @DOC Input writer owning its buffer (and optionally its descriptor).

   Performance:
   - Layout: a ca::io::reader (48 bytes), a ca::fs::descriptor (4) and the allocator (0 for a
     stateless one, 8 for arena_allocator). Reads go through the inner reader: same in-place
     fill/read_until, same costs.
   - The buffer is one allocation from A, 64-byte aligned, made once by the factory; resize()
     allocates the new block before releasing the old and moves only the unread bytes. */
#ifndef ca_resources_io_buffered_reader_hpp
#define ca_resources_io_buffered_reader_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "6-resources/fs/descriptor.hpp"
#include "6-resources/io/reader.hpp"
#include "6-resources/io/writer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- BUFFERED READER ------------------------------------------------

		/* @DOC ca::io::reader whose buffer comes from allocator A, sized by the caller and
		   resizable; over a borrowed descriptor (an int or a ca::fs::descriptor that outlives it)
		   or an adopted one (moved in), closed by cleanup(). Build with over(fd) / adopt(file), or
		   make / make_in(allocator, file, capacity), ca::make_in<buffered_reader<>>(arena, ...). Cleanup is cleanup() (free, close:
		   the close error); the destructor is a failsafe doing the same silently. */
		template <ca::is_allocator A = ca::heap>
		class buffered_reader final {


			private:

				// -- private types ------------------------------------------

				using self = ca::io::buffered_reader<A>;


			public:

				// -- public types -------------------------------------------

				using allocator_type = A;

				template <ca::is_allocator B>
				using rebind = ca::io::buffered_reader<B>;


				// -- public constants ---------------------------------------

				/* @DOC Alignment of the buffer: one cache line. */
				static constexpr ca::usz ALIGNMENT = 64U;


			private:

				// -- private members ----------------------------------------

				ca::fs::descriptor _owned;
				ca::io::reader _reader;
				[[no_unique_address]] A _allocator;


			public:

				// -- public lifecycle ---------------------------------------

				buffered_reader(const self&) = delete;

				buffered_reader(self&& other) noexcept
				: _owned{ca::move(other._owned)}, _reader{ca::move(other._reader)}, _allocator{other._allocator} {
				}

				// Failsafe for a skipped cleanup(): frees and closes, ignoring errors.
				// ~buffered_reader(void) noexcept {
				// 	self::_free(_allocator, _reader.buffer());
				// }

				/* @DOC Over fd, borrowed, with a capacity-byte buffer (> 0) from allocator. */
				[[nodiscard]] static auto make_in(const A& allocator, const int fd, const ca::usz capacity) noexcept -> ca::result<self> {
					return self::_build(allocator, ca::fs::descriptor::adopt(-1), fd, capacity);
				}

				[[nodiscard]] static auto make_in(const A& allocator, const ca::fs::descriptor& file, const ca::usz capacity) noexcept -> ca::result<self> {
					return self::_build(allocator, ca::fs::descriptor::adopt(-1), file.native(), capacity);
				}

				/* @DOC Takes file, closed by cleanup(); on failure file is closed too. */
				[[nodiscard]] static auto make_in(const A& allocator, ca::fs::descriptor&& file, const ca::usz capacity) noexcept -> ca::result<self> {

					const int fd = file.native();
					return self::_build(allocator, ca::move(file), fd, capacity);
				}

				[[nodiscard]] static auto make(const int fd, const ca::usz capacity) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
					return self::make_in(A{}, fd, capacity);
				}

				[[nodiscard]] static auto make(const ca::fs::descriptor& file, const ca::usz capacity) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
					return self::make_in(A{}, file, capacity);
				}

				[[nodiscard]] static auto make(ca::fs::descriptor&& file, const ca::usz capacity) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
					return self::make_in(A{}, ca::move(file), capacity);
				}

				/* @DOC make over a borrowed descriptor, named for it. */
				[[nodiscard]] static auto over(const int fd, const ca::usz capacity) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
					return self::make(fd, capacity);
				}

				[[nodiscard]] static auto over(const ca::fs::descriptor& file, const ca::usz capacity) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
					return self::make(file, capacity);
				}

				/* @DOC make taking file, named for it. */
				[[nodiscard]] static auto adopt(ca::fs::descriptor&& file, const ca::usz capacity) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
					return self::make(ca::move(file), capacity);
				}


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;

				/* @DOC Takes other's buffer and descriptor; the previous buffer is freed. Traps when
				   this one still owns a descriptor, whose close could fail: cleanup() first. */
				auto operator=(self&& other) noexcept -> self& {

					if (this == &other)
						return *this;
					if (_owned.is_open()) [[unlikely]]
						ca::impl::assigned_over_an_uncleaned_object();
					self::_free(_allocator, _reader.buffer());
					ca::destroy_at(&_reader);
					ca::construct_at(&_reader, ca::move(other._reader));
					    _owned = ca::move(other._owned);
					_allocator = other._allocator;
					return *this;
				}


				// -- accessors ----------------------------------------------

				/* @DOC The inner reader, for code taking a ca::io::reader&. */
				[[nodiscard]] constexpr auto reader(void) noexcept -> ca::io::reader& {
					return _reader;
				}

				[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
					return _allocator;
				}

				[[nodiscard]] constexpr auto native(void) const noexcept -> int {
					return _reader.native();
				}

				/* @DOC Whether cleanup() closes the descriptor. */
				[[nodiscard]] constexpr auto owns_descriptor(void) const noexcept -> bool {
					return _owned.is_open();
				}

				[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
					return _reader.capacity();
				}

				[[nodiscard]] constexpr auto available(void) const noexcept -> ca::usz {
					return _reader.available();
				}


				// -- methods ------------------------------------------------

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

				constexpr auto tie(ca::io::writer* const writer) noexcept -> void {
					_reader.tie(writer);
				}

				/* @DOC A capacity-byte buffer (> 0) instead, unread bytes kept. errc::no_space when
				   they do not fit, allocation failure: nothing changes. */
				[[nodiscard]] auto resize(const ca::usz capacity) noexcept -> ca::result<void> {

					if (capacity == _reader.capacity())
						return ca::result<void>{};
					if (capacity == 0U || capacity < _reader.available()) [[unlikely]]
						return ca::failure{capacity == 0U ? ca::errc::invalid_argument : ca::errc::no_space};
					auto block = _allocator.allocate(capacity, ALIGNMENT);
					if (!block) [[unlikely]]
						return block.propagate();
					const ca::span<char> old = _reader.buffer();
					static_cast<void>(_reader.set_buffer(ca::span<char>{static_cast<char*>(block->data()), capacity}));
					self::_free(_allocator, old);
					return ca::result<void>{};
				}

				/* @DOC Frees the buffer, closes an adopted descriptor (its error); the reader is closed
				   afterwards whatever happened. */
				[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

					self::_free(_allocator, _reader.buffer());
					ca::destroy_at(&_reader);
					ca::construct_at(&_reader);
					return _owned.cleanup();
				}


			private:

				// -- private methods ----------------------------------------

				buffered_reader(ca::fs::descriptor&& owned, const int fd, const ca::span<char> buffer, const A& allocator) noexcept
				: _owned{ca::move(owned)}, _reader{fd, buffer}, _allocator{allocator} {
				}

				[[nodiscard]] static auto _build(const A& allocator, ca::fs::descriptor&& owned, const int fd, const ca::usz capacity) noexcept -> ca::result<self> {

					if (capacity == 0U) [[unlikely]] {
						static_cast<void>(owned.cleanup());
						return ca::failure{ca::errc::invalid_argument};
					}
					auto block = allocator.allocate(capacity, ALIGNMENT);
					if (!block) [[unlikely]] {
						static_cast<void>(owned.cleanup());
						return block.propagate();
					}
					return self{ca::move(owned), fd, ca::span<char>{static_cast<char*>(block->data()), capacity}, allocator};
				}

				static auto _free(const A& allocator, const ca::span<char> buffer) noexcept -> void {
					if (buffer.data() != nullptr)
						allocator.deallocate(ca::allocation{buffer.data(), buffer.size()}, ALIGNMENT);
				}

		}; // class buffered_reader

	} // namespace io

} // namespace ca

#endif // ca_resources_io_buffered_reader_hpp
