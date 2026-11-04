#ifndef ca_collections_regex_impl_regex_parser_hpp
#define ca_collections_regex_impl_regex_parser_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "2-vocabulary/unicode/decoded.hpp"
#include "2-vocabulary/unicode/impl/decode_at.hpp"
#include "2-vocabulary/unicode/properties_of.hpp"
#include "5-collections/regex/impl/regex_limits.hpp"
#include "5-collections/regex/impl/regex_look.hpp"
#include "5-collections/regex/regex_error.hpp"
#include "5-collections/regex/regex_flags.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX PARSER -------------------------------------------------------

	namespace impl {

		[[nodiscard]] constexpr auto has_flag(const ca::regex_flags flags, const ca::regex_flags flag) noexcept -> bool {
			return (flags & flag) != ca::regex_flags::none;
		}

		/* @DOC Python's re grammar over UTF-8, one pass, no recursion (a fixed stack of open
		   groups), so it runs in constant evaluation. What it finds goes to Sink, which builds
		   a tree or (regex_pattern's check) nothing: empty, literal, any, look, set_begin,
		   set_range, set_class, set_end, repeat, group, name (false: a duplicate), concat,
		   alternate (over a chain made with link), root, ok (false stops the parse, out of
		   memory). Handles are the Sink's. */
		template <typename Sink>
		class regex_parser final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::regex_parser<Sink>;
				using kind = ca::regex_error::kind;

				enum class item : ca::u8 {
					none,
					atom,
					look,
					repeated,
				};

				struct frame final {

						using self = typename ca::impl::regex_parser<Sink>::frame;

						ca::usz         _open;
						ca::u32         _first;
						ca::u32         _last;
						ca::u32         _before_last;
						ca::u32         _count;
						ca::u32         _alternatives_first;
						ca::u32         _alternatives_last;
						ca::u32         _alternatives;
						ca::u32         _capture;
						ca::regex_flags _flags;
						item            _item;

				}; // struct frame

				struct set_item final {

						using self = typename ca::impl::regex_parser<Sink>::set_item;

						char32_t              _point;
						ca::impl::regex_class _class;
						bool                  _named;
						bool                  _negated;

				}; // struct set_item


			private:

				// -- private members ----------------------------------------

				const char*                   _text;
				ca::usz                       _size;
				ca::usz                       _at;
				Sink&                         _sink;
				ca::u32                       _groups;
				ca::usz                       _depth;
				bool                          _failed;
				ca::regex_error               _error;
				frame                         _frames[ca::impl::regex_max_depth + 1U];


			public:

				// -- public lifecycle ---------------------------------------

				constexpr regex_parser(const ca::sv::str pattern, Sink& sink) noexcept
				: _text{pattern.data()}, _size{pattern.size()}, _at{0U}, _sink{sink}, _groups{0U}, _depth{0U}, _failed{false},
				  _error{kind::missing_paren, 0U}, _frames{} {
				}


				// -- methods ------------------------------------------------

				/* @DOC Parses the whole pattern under flags; the first syntax error, or nothing (the
				   sink saw it all, unless its ok() turned false). */
				[[nodiscard]] constexpr auto run(const ca::regex_flags flags) noexcept -> ca::optional<ca::regex_error> {

					this->_open(0U, 0U, flags);
					while (!_failed && _sink.ok()) {
						this->_skip_verbose();
						if (_at >= _size)
							break;
						this->_step();
					}
					if (_failed)
						return _error;
					if (!_sink.ok())
						return ca::nullopt;
					if (_depth != 0U)
						return ca::regex_error{kind::missing_paren, _frames[_depth]._open};
					_sink.root(this->_close_alternation(_frames[0]));
					return ca::nullopt;
				}

				/* @DOC Capture groups seen. */
				[[nodiscard]] constexpr auto groups(void) const noexcept -> ca::u32 {
					return _groups;
				}


			private:

				// -- private methods ----------------------------------------

				constexpr auto _fail(const kind what, const ca::usz position) noexcept -> void {

					if (_failed)
						return;
					 _error = ca::regex_error{what, position};
					_failed = true;
				}

				[[nodiscard]] constexpr auto _flags(void) const noexcept -> ca::regex_flags {
					return _frames[_depth]._flags;
				}

				[[nodiscard]] constexpr auto _peek(const ca::usz ahead = 0U) const noexcept -> char {
					return _at + ahead < _size ? _text[_at + ahead] : '\0';
				}

				[[nodiscard]] constexpr auto _more(const ca::usz ahead = 0U) const noexcept -> bool {
					return _at + ahead < _size;
				}

				[[nodiscard]] static constexpr auto _is_verbose_space(const char c) noexcept -> bool {
					return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
				}

				[[nodiscard]] static constexpr auto _is_ascii_letter(const char c) noexcept -> bool {
					return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
				}

				[[nodiscard]] static constexpr auto _is_digit(const char c) noexcept -> bool {
					return c >= '0' && c <= '9';
				}

				[[nodiscard]] static constexpr auto _is_octal(const char c) noexcept -> bool {
					return c >= '0' && c <= '7';
				}

				[[nodiscard]] static constexpr auto _hex_value(const char c) noexcept -> ca::u32 {

					if (c >= '0' && c <= '9')
						return static_cast<ca::u32>(c - '0');
					if (c >= 'a' && c <= 'f')
						return static_cast<ca::u32>(c - 'a') + 10U;
					if (c >= 'A' && c <= 'F')
						return static_cast<ca::u32>(c - 'A') + 10U;
					return 16U;
				}

				constexpr auto _skip_verbose(void) noexcept -> void {

					if (!ca::impl::has_flag(this->_flags(), ca::regex_flags::verbose))
						return;
					while (_at < _size) {
						const char c = _text[_at];
						if (self::_is_verbose_space(c))
							++_at;
						else if (c == '#') {
							while (_at < _size && _text[_at] != '\n')
								++_at;
						}
						else
							return;
					}
				}

				[[nodiscard]] constexpr auto _decode(void) noexcept -> char32_t {

					const ca::unicode::decoded point = ca::unicode::impl::decode_at(_text + _at, _text + _size);
					if (!point.valid()) {
						this->_fail(kind::invalid_utf8, _at);
						return U'\0';
					}
					_at += point.length();
					return point.code_point();
				}

				constexpr auto _open(const ca::usz open, const ca::u32 capture, const ca::regex_flags flags) noexcept -> void {

					frame& f = _frames[_depth];
					                 f._open = open;
					                f._first = ca::impl::regex_none;
					                 f._last = ca::impl::regex_none;
					          f._before_last = ca::impl::regex_none;
					                f._count = 0U;
					   f._alternatives_first = ca::impl::regex_none;
					    f._alternatives_last = ca::impl::regex_none;
					         f._alternatives = 0U;
					              f._capture = capture;
					                f._flags = flags;
					                 f._item = item::none;
				}

				constexpr auto _append(const ca::u32 handle, const item what) noexcept -> void {

					frame& f = _frames[_depth];
					if (f._count == 0U) {
						      f._first = handle;
						f._before_last = ca::impl::regex_none;
					}
					else {
						_sink.link(f._last, handle);
						f._before_last = f._last;
					}
					 f._last = handle;
					 f._item = what;
					++f._count;
				}

				[[nodiscard]] constexpr auto _close_branch(frame& f) noexcept -> ca::u32 {

					ca::u32 branch = f._first;
					if (f._count == 0U)
						branch = _sink.empty();
					else if (f._count > 1U)
						branch = _sink.concat(f._first);
					f._count = 0U;
					 f._item = item::none;
					return branch;
				}

				constexpr auto _push_branch(frame& f) noexcept -> void {

					const ca::u32 branch = this->_close_branch(f);
					if (f._alternatives == 0U)
						f._alternatives_first = branch;
					else
						_sink.link(f._alternatives_last, branch);
					f._alternatives_last = branch;
					++f._alternatives;
				}

				[[nodiscard]] constexpr auto _close_alternation(frame& f) noexcept -> ca::u32 {

					this->_push_branch(f);
					if (f._alternatives == 1U)
						return f._alternatives_first;
					return _sink.alternate(f._alternatives_first);
				}

				constexpr auto _step(void) noexcept -> void {

					const char c = _text[_at];
					switch (c) {
						case '|':
							++_at;
							this->_push_branch(_frames[_depth]);
							return;
						case ')':
							this->_close_group();
							return;
						case '(':
							this->_open_group();
							return;
						case '*':
						case '+':
						case '?':
						case '{':
							this->_repeat();
							return;
						case '[':
							this->_set();
							return;
						case '.':
							++_at;
							this->_append(_sink.any(this->_flags()), item::atom);
							return;
						case '^':
							++_at;
							this->_append(_sink.look(ca::impl::has_flag(this->_flags(), ca::regex_flags::multiline)
								? ca::impl::regex_look::begin_line : ca::impl::regex_look::begin_text), item::look);
							return;
						case '$':
							++_at;
							this->_append(_sink.look(ca::impl::has_flag(this->_flags(), ca::regex_flags::multiline)
								? ca::impl::regex_look::end_line : ca::impl::regex_look::end_text_or_newline), item::look);
							return;
						case '\\':
							this->_escape();
							return;
						default:
							break;
					}
					const char32_t point = this->_decode();
					if (!_failed)
						this->_append(_sink.literal(point, this->_flags()), item::atom);
				}

				constexpr auto _close_group(void) noexcept -> void {

					if (_depth == 0U) {
						this->_fail(kind::unbalanced_paren, _at);
						return;
					}
					++_at;
					frame& f = _frames[_depth];
					ca::u32 handle = this->_close_alternation(f);
					if (f._capture != 0U)
						handle = _sink.group(handle, f._capture);
					--_depth;
					this->_append(handle, item::atom);
				}

				constexpr auto _push(const ca::usz open, const ca::u32 capture, const ca::regex_flags flags) noexcept -> void {

					if (_depth == ca::impl::regex_max_depth) {
						this->_fail(kind::too_deep, open);
						return;
					}
					++_depth;
					this->_open(open, capture, flags);
				}

				[[nodiscard]] constexpr auto _new_capture(const ca::usz open) noexcept -> ca::u32 {

					if (_groups == ca::impl::regex_max_groups) {
						this->_fail(kind::too_many_groups, open);
						return 0U;
					}
					return ++_groups;
				}

				constexpr auto _open_group(void) noexcept -> void {

					const ca::usz open = _at++;
					if (this->_peek() != '?') {
						const ca::u32 capture = this->_new_capture(open);
						if (!_failed)
							this->_push(open, capture, this->_flags());
						return;
					}
					++_at;
					if (!this->_more()) {
						this->_fail(kind::missing_paren, open);
						return;
					}
					const char c = _text[_at];
					switch (c) {
						case ':':
							++_at;
							this->_push(open, 0U, this->_flags());
							return;
						case 'P':
							++_at;
							this->_python_group(open);
							return;
						case '#':
							this->_comment(open);
							return;
						case '=':
						case '!':
							this->_fail(kind::lookaround, open);
							return;
						case '<':
							this->_fail(this->_peek(1U) == '=' || this->_peek(1U) == '!' ? kind::lookaround : kind::unknown_extension, open);
							return;
						case '(':
							this->_fail(kind::conditional, open);
							return;
						case '>':
							this->_fail(kind::atomic, open);
							return;
						default:
							break;
					}
					this->_inline_flags(open);
				}

				constexpr auto _python_group(const ca::usz open) noexcept -> void {

					const char c = this->_peek();
					if (c == '=') {
						this->_fail(kind::backreference, open);
						return;
					}
					if (c != '<') {
						this->_fail(kind::unknown_extension, open);
						return;
					}
					++_at;
					const ca::usz start = _at;
					bool first = true;
					while (this->_more() && this->_peek() != '>') {
						const ca::usz here = _at;
						const char32_t point = this->_decode();
						if (_failed)
							return;
						const ca::unicode::properties properties = ca::unicode::properties_of(point);
						if (first ? !properties.is_identifier_start() : !properties.is_identifier_continue()) {
							this->_fail(kind::bad_group_name, here);
							return;
						}
						first = false;
					}
					if (!this->_more()) {
						this->_fail(kind::missing_group_name, start);
						return;
					}
					if (_at == start) {
						this->_fail(kind::missing_group_name, start);
						return;
					}
					const ca::sv::str name{_text + start, _at - start};
					++_at;
					const ca::u32 capture = this->_new_capture(open);
					if (_failed)
						return;
					if (!_sink.name(name, capture)) {
						this->_fail(kind::duplicate_group_name, start);
						return;
					}
					this->_push(open, capture, this->_flags());
				}

				constexpr auto _comment(const ca::usz open) noexcept -> void {

					while (this->_more() && this->_peek() != ')')
						++_at;
					if (!this->_more()) {
						this->_fail(kind::unterminated_comment, open);
						return;
					}
					++_at;
				}

				[[nodiscard]] static constexpr auto _flag_of(const char c) noexcept -> ca::regex_flags {
					switch (c) {
						case 'i':
							return ca::regex_flags::ignore_case;
						case 'm':
							return ca::regex_flags::multiline;
						case 's':
							return ca::regex_flags::dot_all;
						case 'x':
							return ca::regex_flags::verbose;
						case 'a':
							return ca::regex_flags::ascii;
						default:
							return ca::regex_flags::none;
					}
				}

				constexpr auto _inline_flags(const ca::usz open) noexcept -> void {

					if (this->_peek() == ')') {
						this->_fail(kind::unknown_extension, open);
						return;
					}
					ca::regex_flags added = ca::regex_flags::none;
					ca::regex_flags removed = ca::regex_flags::none;
					bool unicode = false;
					while (this->_more() && this->_peek() != '-' && this->_peek() != ':' && this->_peek() != ')') {
						const char c = _text[_at];
						if (c == 'u')
							unicode = true;
						else if (self::_flag_of(c) != ca::regex_flags::none)
							added = added | self::_flag_of(c);
						else {
							this->_fail(c == 'L' ? kind::bad_flags : (_at == open + 2U ? kind::unknown_extension : kind::bad_flags), _at);
							return;
						}
						++_at;
					}
					const ca::regex_flags current = this->_flags();
					if (unicode && (ca::impl::has_flag(added, ca::regex_flags::ascii) || ca::impl::has_flag(current, ca::regex_flags::ascii))) {
						this->_fail(kind::ascii_and_unicode, open);
						return;
					}
					if (this->_peek() == '-') {
						++_at;
						if (this->_peek() == ':') {
							this->_fail(kind::bad_flags, _at);
							return;
						}
						while (this->_more() && this->_peek() != ':') {
							const ca::regex_flags flag = self::_flag_of(_text[_at]);
							if (flag == ca::regex_flags::none || flag == ca::regex_flags::ascii) {
								this->_fail(kind::bad_flags, _at);
								return;
							}
							removed = removed | flag;
							++_at;
						}
						if (!this->_more()) {
							this->_fail(kind::missing_paren, open);
							return;
						}
					}
					if (!this->_more()) {
						this->_fail(kind::missing_paren, open);
						return;
					}
					if (_text[_at] == ')') {
						++_at;
						frame& top = _frames[0];
						if (_depth != 0U || top._alternatives != 0U || top._count != 0U) {
							this->_fail(kind::global_flags_not_at_start, open);
							return;
						}
						top._flags = top._flags | added;
						return;
					}
					++_at;
					this->_push(open, 0U, (current | added) & ~removed);
				}

				[[nodiscard]] constexpr auto _number(ca::u32& value) noexcept -> bool {

					const ca::usz start = _at;
					value = 0U;
					while (this->_more() && self::_is_digit(this->_peek())) {
						if (value <= ca::impl::regex_max_repeat)
							value = value * 10U + static_cast<ca::u32>(_text[_at] - '0');
						++_at;
					}
					return _at != start;
				}

				constexpr auto _repeat(void) noexcept -> void {

					const ca::usz start = _at;
					const char c = _text[_at++];
					ca::u32 min = 0U;
					ca::u32 max = ca::impl::regex_infinite;
					if (c == '+')
						min = 1U;
					else if (c == '?')
						max = 1U;
					else if (c == '{') {
						if (this->_peek() == '}') {
							this->_append(_sink.literal(U'{', this->_flags()), item::atom);
							return;
						}
						const bool low = this->_number(min);
						bool high = false;
						if (this->_peek() == ',') {
							++_at;
							high = this->_number(max);
							if (!high)
								max = ca::impl::regex_infinite;
						}
						else {
							  max = min;
							 high = low;
						}
						if (this->_peek() != '}') {
							_at = start + 1U;
							this->_append(_sink.literal(U'{', this->_flags()), item::atom);
							return;
						}
						++_at;
						if (!low)
							min = 0U;
						if (min > ca::impl::regex_max_repeat || (high && max > ca::impl::regex_max_repeat)) {
							this->_fail(kind::repeat_too_large, start);
							return;
						}
						if (high && max < min) {
							this->_fail(kind::min_over_max, start);
							return;
						}
					}
					frame& f = _frames[_depth];
					if (f._item == item::none || f._item == item::look) {
						this->_fail(kind::nothing_to_repeat, start);
						return;
					}
					if (f._item == item::repeated) {
						this->_fail(kind::multiple_repeat, start);
						return;
					}
					bool greedy = true;
					if (this->_peek() == '?') {
						++_at;
						greedy = false;
					}
					else if (this->_peek() == '+') {
						this->_fail(kind::atomic, _at);
						return;
					}
					const ca::u32 handle = _sink.repeat(f._last, min, max, greedy);
					if (f._before_last == ca::impl::regex_none)
						f._first = handle;
					else
						_sink.link(f._before_last, handle);
					f._last = handle;
					f._item = item::repeated;
				}

				[[nodiscard]] constexpr auto _hex(const ca::usz digits, const ca::usz start) noexcept -> char32_t {

					ca::u32 value = 0U;
					for (ca::usz i = 0U; i < digits; ++i) {
						const ca::u32 digit = self::_hex_value(this->_peek());
						if (!this->_more() || digit == 16U) {
							this->_fail(kind::bad_escape, start);
							return U'\0';
						}
						value = value * 16U + digit;
						++_at;
					}
					if (value > 0x10FFFFU || (value >= 0xD800U && value <= 0xDFFFU)) {
						this->_fail(kind::bad_code_point, start);
						return U'\0';
					}
					return static_cast<char32_t>(value);
				}

				[[nodiscard]] constexpr auto _octal(const ca::usz start) noexcept -> char32_t {

					ca::u32 value = 0U;
					for (ca::usz i = 0U; i < 3U && this->_more() && self::_is_octal(this->_peek()); ++i)
						value = value * 8U + static_cast<ca::u32>(_text[_at++] - '0');
					if (value > 0377U)
						this->_fail(kind::bad_escape, start);
					return static_cast<char32_t>(value);
				}

				[[nodiscard]] static constexpr auto _control(const char c) noexcept -> char32_t {
					switch (c) {
						case 'a':
							return U'\a';
						case 'f':
							return U'\f';
						case 'n':
							return U'\n';
						case 'r':
							return U'\r';
						case 't':
							return U'\t';
						case 'v':
							return U'\v';
						default:
							return U'\0';
					}
				}

				[[nodiscard]] static constexpr auto _named(const char c, set_item& out) noexcept -> bool {

					switch (c) {
						case 'd':
						case 'D':
							out._class = ca::impl::regex_class::digit;
							break;
						case 'w':
						case 'W':
							out._class = ca::impl::regex_class::word;
							break;
						case 's':
						case 'S':
							out._class = ca::impl::regex_class::space;
							break;
						default:
							return false;
					}
					  out._named = true;
					out._negated = c >= 'A' && c <= 'Z';
					return true;
				}

				[[nodiscard]] constexpr auto _common_escape(const char c, const ca::usz start, set_item& out) noexcept -> bool {

					if (self::_named(c, out))
						return true;
					if (self::_control(c) != U'\0') {
						out._point = self::_control(c);
						return true;
					}
					switch (c) {
						case 'x':
							out._point = this->_hex(2U, start);
							return true;
						case 'u':
							out._point = this->_hex(4U, start);
							return true;
						case 'U':
							out._point = this->_hex(8U, start);
							return true;
						case 'N':
							this->_fail(kind::named_character, start);
							return true;
						default:
							return false;
					}
				}

				[[nodiscard]] constexpr auto _escape_item(const bool in_set, set_item& out) noexcept -> bool {

					const ca::usz start = _at++;
					if (!this->_more()) {
						this->_fail(kind::trailing_backslash, start);
						return false;
					}
					const char c = _text[_at];
					out = set_item{U'\0', ca::impl::regex_class::word, false, false};
					if (self::_is_ascii_letter(c) || self::_is_digit(c)) {
						++_at;
						if (this->_common_escape(c, start, out))
							return !_failed;
						if (in_set && c == 'b') {
							out._point = U'\b';
							return true;
						}
						if (c == '0' || (in_set && self::_is_octal(c))) {
							--_at;
							out._point = this->_octal(start);
							return !_failed;
						}
						if (!in_set && self::_is_digit(c)) {
							if (self::_is_octal(c) && self::_is_octal(this->_peek()) && self::_is_octal(this->_peek(1U))) {
								--_at;
								out._point = this->_octal(start);
								return !_failed;
							}
							this->_fail(kind::backreference, start);
							return false;
						}
						this->_fail(kind::bad_escape, start);
						return false;
					}
					out._point = this->_decode();
					return !_failed;
				}

				constexpr auto _escape(void) noexcept -> void {

					const char c = this->_peek(1U);
					ca::impl::regex_look look = ca::impl::regex_look::begin_text;
					const bool ascii = ca::impl::has_flag(this->_flags(), ca::regex_flags::ascii);
					bool is_look = true;
					switch (c) {
						case 'A':
							look = ca::impl::regex_look::begin_text;
							break;
						case 'Z':
							look = ca::impl::regex_look::end_text;
							break;
						case 'b':
							look = ascii ? ca::impl::regex_look::word_ascii : ca::impl::regex_look::word;
							break;
						case 'B':
							look = ascii ? ca::impl::regex_look::not_word_ascii : ca::impl::regex_look::not_word;
							break;
						default:
							is_look = false;
							break;
					}
					if (is_look) {
						_at += 2U;
						this->_append(_sink.look(look), item::look);
						return;
					}
					set_item escaped{U'\0', ca::impl::regex_class::word, false, false};
					if (!this->_escape_item(false, escaped))
						return;
					if (!escaped._named) {
						this->_append(_sink.literal(escaped._point, this->_flags()), item::atom);
						return;
					}
					_sink.set_begin();
					_sink.set_class(escaped._class, escaped._negated, this->_flags());
					this->_append(_sink.set_end(false, this->_flags()), item::atom);
				}

				[[nodiscard]] constexpr auto _set_item(set_item& out) noexcept -> bool {

					if (this->_peek() == '\\')
						return this->_escape_item(true, out);
					out = set_item{this->_decode(), ca::impl::regex_class::word, false, false};
					return !_failed;
				}

				constexpr auto _add(const set_item& one) noexcept -> void {

					if (one._named)
						_sink.set_class(one._class, one._negated, this->_flags());
					else
						_sink.set_range(one._point, one._point);
				}

				constexpr auto _set(void) noexcept -> void {

					const ca::usz start = _at++;
					bool negated = false;
					if (this->_peek() == '^') {
						 negated = true;
						++_at;
					}
					_sink.set_begin();
					ca::usz items = 0U;
					while (true) {
						if (!this->_more()) {
							this->_fail(kind::unterminated_set, start);
							return;
						}
						if (this->_peek() == ']' && items != 0U) {
							++_at;
							break;
						}
						const ca::usz first_at = _at;
						set_item first{U'\0', ca::impl::regex_class::word, false, false};
						if (!this->_set_item(first))
							return;
						++items;
						if (this->_peek() != '-') {
							this->_add(first);
							continue;
						}
						++_at;
						if (!this->_more()) {
							this->_fail(kind::unterminated_set, start);
							return;
						}
						if (this->_peek() == ']') {
							++_at;
							this->_add(first);
							_sink.set_range(U'-', U'-');
							break;
						}
						set_item last{U'\0', ca::impl::regex_class::word, false, false};
						if (!this->_set_item(last))
							return;
						if (first._named || last._named || last._point < first._point) {
							this->_fail(kind::bad_range, first_at);
							return;
						}
						_sink.set_range(first._point, last._point);
					}
					this->_append(_sink.set_end(negated, this->_flags()), item::atom);
				}

		}; // class regex_parser

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_regex_parser_hpp
