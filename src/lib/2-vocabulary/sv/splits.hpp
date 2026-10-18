#ifndef ca_vocabulary_sv_splits_hpp
#define ca_vocabulary_sv_splits_hpp

#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/equal_units.hpp"
#include "1-primitives/memory/find_unit.hpp"
#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/sv/impl/classify.hpp"
#include "2-vocabulary/sv/impl/forward.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SV -----------------------------------------------------------------

	namespace sv {

		// -- SPLITS ---------------------------------------------------------

		/* @DOC The pieces of a split, computed while iterating, nothing allocated: `for (sv::str
		   field : line.split(","))`. Python's rules: split(sep) keeps empty pieces and gives one
		   piece for empty text; split() cuts on runs of white space and drops empty pieces;
		   split_lines() cuts on line boundaries ("\r\n" is one), no piece after a final one. With
		   max, at most max cuts, the rest whole in the last piece. An empty separator does not
		   cut. ca::collect turns it into a dynamic_array. */
		template <ca::is_character C>
		class splits final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sv::splits<C>;


			public:

				// -- public types -------------------------------------------

				/* @DOC How the text is cut. */
				enum class mode : ca::u8 {
					separator,
					white_space,
					lines,
					lines_kept,
				};

				class iterator final {


					private:

						// -- private types ----------------------------------

						using self = typename ca::sv::splits<C>::iterator;


						// -- private members --------------------------------

						const C* _at;
						const C* _end;
						const C* _separator;
						ca::usz _separator_size;
						ca::usz _left;
						const C* _piece;
						ca::usz _piece_size;
						typename ca::sv::splits<C>::mode _mode;
						bool _more;
						bool _done;


					public:

						// -- public lifecycle -------------------------------

						constexpr iterator(const C* const at, const C* const end, const C* const separator, const ca::usz separator_size, const ca::usz left, const typename ca::sv::splits<C>::mode how, const bool done) noexcept
						: _at{at}, _end{end}, _separator{separator}, _separator_size{separator_size}, _left{left}, _piece{at}, _piece_size{0U}, _mode{how}, _more{true}, _done{done} {
							if (!done)
								this->_next();
						}


						// -- accessors --------------------------------------

						[[nodiscard]] constexpr auto operator*(void) const noexcept -> ca::sv::basic<C> {
							return ca::sv::basic<C>{_piece, _piece_size};
						}


						// -- methods ----------------------------------------

						constexpr auto operator++(void) noexcept -> self& {

							this->_next();
							return *this;
						}

						[[nodiscard]] constexpr auto operator==(const self& other) const noexcept -> bool {
							return _done == other._done && (_done || _piece == other._piece);
						}


					private:

						// -- private methods --------------------------------

						constexpr auto _cut(const C* const piece_end, const C* const next) noexcept -> void {

							     _piece = _at;
							_piece_size = static_cast<ca::usz>(piece_end - _at);
							        _at = next;
						}

						constexpr auto _rest(void) noexcept -> void {

							this->_cut(_end, _end);
							_more = false;
						}

						constexpr auto _next(void) noexcept -> void {

							using how = typename ca::sv::splits<C>::mode;
							if (_mode == how::white_space) {
								while (_at != _end) {
									const ca::u8 space = ca::sv::impl::space_at(_at, _end);
									if (space == 0U)
										break;
									_at += space;
								}
							}
							if (!_more || (_at == _end && _mode != how::separator)) {
								_done = true;
								return;
							}
							if (_left == 0U) {
								this->_rest();
								return;
							}
							--_left;
							if (_mode == how::separator)
								this->_next_separator();
							else if (_mode == how::white_space)
								this->_next_white_space();
							else
								this->_next_line();
						}

						constexpr auto _next_separator(void) noexcept -> void {

							if (_separator_size == 1U) {
								const C* const hit = ca::find_unit(_at, _separator[0], static_cast<ca::usz>(_end - _at));
								if (hit != nullptr) {
									this->_cut(hit, hit + 1);
									return;
								}
							}
							else if (_separator_size != 0U && static_cast<ca::usz>(_end - _at) >= _separator_size) {
								const C* const last = _end - _separator_size;
								const C* from = _at;
								while (from <= last) {
									const C* const hit = ca::find_unit(from, _separator[0], static_cast<ca::usz>(last - from) + 1U);
									if (hit == nullptr)
										break;
									if (ca::equal_units(hit, _separator, _separator_size)) {
										this->_cut(hit, hit + _separator_size);
										return;
									}
									from = hit + 1;
								}
							}
							this->_rest();
						}

						constexpr auto _next_white_space(void) noexcept -> void {

							const C* scan = _at;
							while (scan != _end) {
								const ca::u8 space = ca::sv::impl::space_at(scan, _end);
								if (space != 0U) {
									this->_cut(scan, scan + space);
									return;
								}
								scan += ca::sv::impl::point_at(scan, _end).length();
							}
							this->_rest();
						}

						constexpr auto _next_line(void) noexcept -> void {

							const C* scan = _at;
							while (scan != _end) {
								const ca::u8 boundary = ca::sv::impl::line_break_at(scan, _end);
								if (boundary != 0U) {
									this->_cut(_mode == ca::sv::splits<C>::mode::lines_kept ? scan + boundary : scan, scan + boundary);
									return;
								}
								scan += ca::sv::impl::point_at(scan, _end).length();
							}
							this->_rest();
						}

				}; // class iterator


			private:

				// -- private members ----------------------------------------

				const C* _data;
				ca::usz _size;
				const C* _separator;
				ca::usz _separator_size;
				ca::usz _max;
				mode _mode;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr splits(const C* const data, const ca::usz size, const C* const separator, const ca::usz separator_size, const ca::usz max, const mode how) noexcept
				: _data{data}, _size{size}, _separator{separator}, _separator_size{separator_size}, _max{max}, _mode{how} {
				}


				// -- iterators ----------------------------------------------

				[[nodiscard]] constexpr auto begin(void) const noexcept -> iterator {
					return iterator{_data, _data + _size, _separator, _separator_size, _max, _mode, false};
				}

				[[nodiscard]] constexpr auto end(void) const noexcept -> iterator {
					return iterator{_data + _size, _data + _size, _separator, _separator_size, 0U, _mode, true};
				}

		}; // class splits

	} // namespace sv

} // namespace ca

#endif // ca_vocabulary_sv_splits_hpp
