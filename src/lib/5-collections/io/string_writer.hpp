#ifndef ca_collections_io_string_writer_hpp
#define ca_collections_io_string_writer_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_same.hpp"
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


		// -- STRING WRITER --------------------------------------------------

		namespace impl {

			/* @DOC A char string a string_writer can append to: basic_str<char, A>,
			   inline_str<N>. */
			template <typename S>
			concept is_char_string = ca::is_same<typename S::value_type, char> && requires (S& string, const ca::sv::str text, const ca::usz size) {
				{ string.data() } -> ca::is_same<char*>;
				{ string.size() } -> ca::is_same<ca::usz>;
				{ string.capacity() } -> ca::is_same<ca::usz>;
				string.set_size(size);
				{ string.append(text) } -> ca::is_same<ca::result<void>>;
			};

		} // namespace impl


		/* @DOC Writer appending to a char string (ca::str, any basic_str<char, A>,
		   inline_str<N>), lending its spare capacity so formatting writes in place. A growable
		   string doubles when full; when growth fails, or an inline_str is full, the message is
		   dropped whole, the string left as before it: the call fails (out of memory,
		   errc::no_space for inline_str), the error also kept sticky. The string must outlive the
		   writer. */
		template <ca::io::impl::is_char_string S>
		class string_writer final {


			private:

				// -- private types ------------------------------------------

				using self = ca::io::string_writer<S>;


				// -- private members ----------------------------------------

				S* _string;
				ca::usz _mark;
				ca::error _error;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr explicit string_writer(S& string) noexcept
				: _string{&string}, _mark{0U}, _error{} {
				}

				string_writer(const self&) = delete;


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto string(void) const noexcept -> S& {
					return *_string;
				}

				/* @DOC The sticky error, ok when none; kept until flush() or clear_error(). */
				[[nodiscard]] constexpr auto error(void) const noexcept -> ca::error {
					return _error;
				}


				// -- methods ------------------------------------------------

				/* @DOC Appends bytes, or nothing on failure. */
				[[nodiscard]] auto write(const ca::span<const ca::byte> bytes) noexcept -> ca::result<void> {

					auto appended = _string->append(ca::sv::str{reinterpret_cast<const char*>(bytes.data()), bytes.size()});
					if (!appended) [[unlikely]]
						return this->_fail(appended.error());
					return appended;
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

				/* @DOC The spare capacity, where a message starts. */
				[[nodiscard]] auto window(void) noexcept -> ca::span<char> {

					_mark = _string->size();
					return this->_spare();
				}

				/* @DOC Keeps the text up to at, then grows the string and lends the new spare
				   capacity; on failure (inline_str: errc::no_space) the message is dropped. */
				[[nodiscard]] auto drain(char* const at) noexcept -> ca::result<ca::span<char>> {

					_string->set_size(static_cast<ca::usz>(at - _string->data()));
					if constexpr (requires (S& string, const ca::usz count) { string.reserve(count); }) {
						const ca::usz capacity = _string->capacity();
						auto grown = _string->reserve(capacity + (capacity > 64U ? capacity : 64U));
						if (grown) [[likely]]
							return this->_spare();
						_string->set_size(_mark);
						return this->_fail(grown.error());
					}
					else {
						_string->set_size(_mark);
						return this->_fail(ca::errc::no_space);
					}
				}

				[[nodiscard]] constexpr auto settle(char* const at) noexcept -> ca::result<void> {

					_string->set_size(static_cast<ca::usz>(at - _string->data()));
					return ca::result<void>{};
				}


			private:

				// -- private methods ----------------------------------------

				[[nodiscard]] auto _spare(void) const noexcept -> ca::span<char> {

					char* const data = _string->data();
					const ca::usz size = _string->size();
					return ca::span<char>{data + size, _string->capacity() - size};
				}

				[[nodiscard]] constexpr auto _fail(const ca::error error) noexcept -> ca::failure<ca::error> {

					if (_error.ok())
						_error = error;
					return ca::failure{error};
				}

		}; // class string_writer

	} // namespace io

} // namespace ca

#endif // ca_collections_io_string_writer_hpp
