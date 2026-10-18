#ifndef ca_vocabulary_unicode_code_points_hpp
#define ca_vocabulary_unicode_code_points_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/string_length.hpp"
#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/sv/impl/forward.hpp"
#include "2-vocabulary/unicode/decoded.hpp"
#include "2-vocabulary/unicode/impl/decode_at.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- CODE POINTS ----------------------------------------------------

		/* @DOC The code points of text in C's encoding, for range-for: `for (char32_t c :
		   ca::unicode::code_points{text})`. Invalid sequences yield U+FFFD; the iterator's
		   decoded() and position() tell them apart and locate them. */
		template <ca::is_character C>
		class code_points final {


			private:

				// -- private types ------------------------------------------

				using self = ca::unicode::code_points<C>;


			public:

				// -- public types -------------------------------------------

				class iterator final {


					private:

						// -- private types ----------------------------------

						using self = typename ca::unicode::code_points<C>::iterator;


						// -- private members --------------------------------

						const C* _at;
						const C* _end;
						ca::unicode::decoded _current;


					public:

						// -- public lifecycle -------------------------------

						constexpr iterator(const C* const at, const C* const end) noexcept
						: _at{at}, _end{end}, _current{self::_decode(at, end)} {
						}


						// -- accessors --------------------------------------

						[[nodiscard]] constexpr auto operator*(void) const noexcept -> char32_t {
							return _current.code_point();
						}

						/* @DOC The current code point with its length and validity. */
						[[nodiscard]] constexpr auto decoded(void) const noexcept -> ca::unicode::decoded {
							return _current;
						}

						/* @DOC Where the current code point starts. */
						[[nodiscard]] constexpr auto position(void) const noexcept -> const C* {
							return _at;
						}


						// -- methods ----------------------------------------

						constexpr auto operator++(void) noexcept -> self& {

							      _at += _current.length();
							_current = self::_decode(_at, _end);
							return *this;
						}

						[[nodiscard]] constexpr auto operator==(const self& other) const noexcept -> bool {
							return _at == other._at;
						}


					private:

						// -- private methods --------------------------------

						[[nodiscard]] static constexpr auto _decode(const C* const at, const C* const end) noexcept -> ca::unicode::decoded {
							return at != end ? ca::unicode::impl::decode_at(at, end) : ca::unicode::decoded{U'\0', 0U, false};
						}

				}; // class iterator


			private:

				// -- private members ----------------------------------------

				const C* _data;
				ca::usz _size;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr code_points(const C* const data, const ca::usz size) noexcept
				: _data{data}, _size{size} {
				}

				/* @DOC A NUL-terminated string. */
				constexpr code_points(const C* const text) noexcept
				: _data{text}, _size{ca::string_length(text)} {
				}

				constexpr code_points(const ca::sv::basic<C> text) noexcept
				: _data{text.data()}, _size{text.size()} {
				}


				// -- iterators ----------------------------------------------

				[[nodiscard]] constexpr auto begin(void) const noexcept -> iterator {
					return iterator{_data, _data + _size};
				}

				[[nodiscard]] constexpr auto end(void) const noexcept -> iterator {
					return iterator{_data + _size, _data + _size};
				}

		}; // class code_points

		template <ca::is_character C>
		code_points(const C*) -> code_points<C>;

		template <ca::is_character C>
		code_points(ca::sv::basic<C>) -> code_points<C>;

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_code_points_hpp
