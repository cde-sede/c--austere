#ifndef ca_collections_format_text_writer_hpp
#define ca_collections_format_text_writer_hpp

#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/fill_bytes.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TEXT WRITER --------------------------------------------------------

	/* @DOC Cursor print formats through, into a window lent by a writer (or a stack stage),
	   refilled by drain when full. A type is displayable when it has
	   `auto display(ca::text_writer& out) const noexcept -> void`, which writes with put() or
	   ca::format_to(out, ...). A failed drain ends the message: later writes are ignored and
	   error() holds the failure, which the outer format_to or print returns. */
	class text_writer final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::text_writer;


		public:

			// -- public types -----------------------------------------------

			/* @DOC Takes the full window ending at at; returns fresh space, never empty, or the
			   error that drops the rest of the message. */
			using drain_function = auto (*)(void* target, char* at) noexcept -> ca::result<ca::span<char>>;


		private:

			// -- private members --------------------------------------------

			char* _at;
			char* _end;
			void* _target;
			drain_function _drain;
			ca::error _error;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr text_writer(const ca::span<char> window, void* const target, const drain_function drain) noexcept
			: _at{window.begin()}, _end{window.end()}, _target{target}, _drain{drain}, _error{} {
			}

			text_writer(const self&) = delete;


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;


			// -- accessors --------------------------------------------------

			/* @DOC Where the next char goes. */
			[[nodiscard]] constexpr auto position(void) const noexcept -> char* {
				return _at;
			}

			/* @DOC Why the message was cut, ok when it was not. */
			[[nodiscard]] constexpr auto error(void) const noexcept -> ca::error {
				return _error;
			}


			// -- methods ----------------------------------------------------

			auto put(const char c) noexcept -> void {
				if (_at == _end) [[unlikely]] {
					this->_put_slow(&c, 1U);
					return;
				}
				*_at++ = c;
			}

			auto put(const ca::sv::str text) noexcept -> void {

				const ca::usz size = text.size();
				if (size > static_cast<ca::usz>(_end - _at)) [[unlikely]] {
					this->_put_slow(text.data(), size);
					return;
				}
				ca::copy_bytes(_at, text.data(), size);
				_at += size;
			}

			/* @DOC count copies of c. */
			auto fill(const char c, const ca::usz count) noexcept -> void {

				if (count > static_cast<ca::usz>(_end - _at)) [[unlikely]] {
					this->_fill_slow(c, count);
					return;
				}
				ca::fill_bytes(_at, static_cast<ca::u8>(c), count);
				_at += count;
			}


		private:

			// -- private methods --------------------------------------------

			/* @DOC Drains the full window; false when the rest of the message is dropped. */
			[[nodiscard]] auto _refill(void) noexcept -> bool;

			auto _put_slow(const char* data, ca::usz size) noexcept -> void;

			auto _fill_slow(char c, ca::usz count) noexcept -> void;

	}; // class text_writer

} // namespace ca

#endif // ca_collections_format_text_writer_hpp
