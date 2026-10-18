#ifndef ca_collections_io_memory_writer_hpp
#define ca_collections_io_memory_writer_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- MEMORY WRITER --------------------------------------------------

		/* @DOC Writer into fixed memory the caller provides: format_to's snprintf. A message (or
		   write) that does not fit is dropped whole: the call fails with errc::no_space, also
		   kept sticky. No NUL is added. */
		class memory_writer final {


			private:

				// -- private types ------------------------------------------

				using self = ca::io::memory_writer;


				// -- private members ----------------------------------------

				char* _begin;
				char* _at;
				char* _end;
				ca::error _error;


			public:

				// -- public lifecycle ---------------------------------------

				/* @DOC Writes into buffer (a char array, or ca::span{data, capacity}). */
				constexpr memory_writer(const ca::span<char> buffer) noexcept
				: _begin{buffer.begin()}, _at{buffer.begin()}, _end{buffer.end()}, _error{} {
				}

				memory_writer(const self&) = delete;


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;


				// -- accessors ----------------------------------------------

				/* @DOC The text written so far. */
				[[nodiscard]] constexpr auto view(void) const noexcept -> ca::sv::str {
					return ca::sv::str{_begin, static_cast<ca::usz>(_at - _begin)};
				}

				[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
					return static_cast<ca::usz>(_at - _begin);
				}

				[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
					return static_cast<ca::usz>(_end - _begin);
				}

				/* @DOC The sticky error, ok when none; kept until flush(), clear_error() or clear(). */
				[[nodiscard]] constexpr auto error(void) const noexcept -> ca::error {
					return _error;
				}


				// -- methods ------------------------------------------------

				/* @DOC Appends bytes when they fit, else nothing (errc::no_space). */
				[[nodiscard]] constexpr auto write(const ca::span<const ca::byte> bytes) noexcept -> ca::result<void> {

					const ca::usz size = bytes.size();
					if (size > static_cast<ca::usz>(_end - _at)) [[unlikely]]
						return this->_fail(ca::errc::no_space);
					ca::copy_bytes(_at, bytes.data(), size);
					_at += size;
					return ca::result<void>{};
				}

				/* @DOC Forgets the sticky error, once handled. */
				constexpr auto clear_error(void) noexcept -> void {
					_error = ca::error{};
				}

				/* @DOC Nothing to write out: returns the sticky error and clears it. */
				[[nodiscard]] constexpr auto flush(void) noexcept -> ca::result<void> {

					const ca::error error = ca::exchange(_error, ca::error{});
					if (!error.ok())
						return ca::failure{error};
					return ca::result<void>{};
				}

				/* @DOC Empties the text and clears the sticky error. */
				constexpr auto clear(void) noexcept -> void {

					   _at = _begin;
					_error = ca::error{};
				}

				[[nodiscard]] constexpr auto window(void) const noexcept -> ca::span<char> {
					return ca::span<char>{_at, static_cast<ca::usz>(_end - _at)};
				}

				/* @DOC Full: the message is dropped, errc::no_space. */
				[[nodiscard]] constexpr auto drain(char* const) noexcept -> ca::result<ca::span<char>> {
					return this->_fail(ca::errc::no_space);
				}

				[[nodiscard]] constexpr auto settle(char* const at) noexcept -> ca::result<void> {

					_at = at;
					return ca::result<void>{};
				}


			private:

				// -- private methods ----------------------------------------

				[[nodiscard]] constexpr auto _fail(const ca::error error) noexcept -> ca::failure<ca::error> {

					if (_error.ok())
						_error = error;
					return ca::failure{error};
				}

		}; // class memory_writer

	} // namespace io

} // namespace ca

#endif // ca_collections_io_memory_writer_hpp
