#ifndef ca_vocabulary_sv_impl_methods_hpp
#define ca_vocabulary_sv_impl_methods_hpp

#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/uint_of.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/compare_units.hpp"
#include "1-primitives/memory/equal_units.hpp"
#include "1-primitives/memory/find_unit.hpp"
#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/sv/impl/classify.hpp"
#include "2-vocabulary/sv/impl/forward.hpp"
#include "2-vocabulary/sv/partitioned.hpp"
#include "2-vocabulary/sv/splits.hpp"
#include "2-vocabulary/unicode/code_points.hpp"
#include "2-vocabulary/unicode/count_code_points.hpp"
#include "2-vocabulary/unicode/decoded.hpp"
#include "2-vocabulary/unicode/impl/char_record.hpp"
#include "2-vocabulary/unicode/impl/decode_at.hpp"
#include "2-vocabulary/unicode/impl/flags_of.hpp"
#include "2-vocabulary/unicode/properties.hpp"
#include "2-vocabulary/unicode/properties_of.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SV -----------------------------------------------------------------

	namespace sv {

		// -- METHODS --------------------------------------------------------

		namespace impl {

			/* @DOC The read-only text methods, Python's str API where it fits, written once for
			   every type with view() -> sv::basic<C>: sv::basic, basic_str, inline_str. They
			   return views into the text and never allocate. Indices and sizes count code units;
			   classes and white space are per code point (ca::unicode::properties). Names that
			   differ from Python: lstrip/rstrip strip_start/strip_end, removeprefix/removesuffix
			   strip_prefix/strip_suffix, rfind find_last, rpartition partition_last, splitlines
			   split_lines, isX is_X, len() code_point_count (size() counts units); the
			   allocating ones are on basic_str (ljust/rjust justify_left/justify_right, zfill
			   zero_fill, swapcase swap_case, casefold case_fold, expandtabs expand_tabs, s * n
			   repeat), sep.join ca::join, list() ca::collect. */
			template <typename T, ca::is_character C> // final: ignore
			class methods {


				private:

					// -- private types --------------------------------------

					using self = ca::sv::impl::methods<T, C>;

					using view_type = ca::sv::basic<C>;


				public:

					// -- cutting --------------------------------------------

					/* @DOC First count units, or all of them. */
					[[nodiscard]] constexpr auto prefix(const ca::usz count) const noexcept -> view_type {

						const view_type text = this->_view();
						return view_type{text.data(), count < text.size() ? count : text.size()};
					}

					/* @DOC Last count units, or all of them. */
					[[nodiscard]] constexpr auto suffix(const ca::usz count) const noexcept -> view_type {

						const view_type text = this->_view();
						const ca::usz kept = count < text.size() ? count : text.size();
						return view_type{text.data() + text.size() - kept, kept};
					}

					/* @DOC All but the first count units; empty when count >= size(). */
					[[nodiscard]] constexpr auto without_prefix(const ca::usz count) const noexcept -> view_type {

						const view_type text = this->_view();
						const ca::usz dropped = count < text.size() ? count : text.size();
						return view_type{text.data() + dropped, text.size() - dropped};
					}

					/* @DOC All but the last count units; empty when count >= size(). */
					[[nodiscard]] constexpr auto without_suffix(const ca::usz count) const noexcept -> view_type {

						const view_type text = this->_view();
						return view_type{text.data(), count < text.size() ? text.size() - count : 0U};
					}

					/* @DOC Up to count units from offset, both clamped: s[offset:offset + count]. */
					[[nodiscard]] constexpr auto slice(const ca::usz offset, const ca::usz count) const noexcept -> view_type {
						return this->without_prefix(offset).prefix(count);
					}

					/* @DOC Without leading and trailing white space (str.strip), or without the leading
					   and trailing code points found in chars (str.strip(chars)). */
					[[nodiscard]] constexpr auto strip(void) const noexcept -> view_type {
						return self::_strip_end(self::_strip_start(this->_view()));
					}

					[[nodiscard]] constexpr auto strip(const view_type chars) const noexcept -> view_type {
						return self::_strip_end(self::_strip_start(this->_view(), chars), chars);
					}

					/* @DOC str.lstrip. */
					[[nodiscard]] constexpr auto strip_start(void) const noexcept -> view_type {
						return self::_strip_start(this->_view());
					}

					[[nodiscard]] constexpr auto strip_start(const view_type chars) const noexcept -> view_type {
						return self::_strip_start(this->_view(), chars);
					}

					/* @DOC str.rstrip. */
					[[nodiscard]] constexpr auto strip_end(void) const noexcept -> view_type {
						return self::_strip_end(this->_view());
					}

					[[nodiscard]] constexpr auto strip_end(const view_type chars) const noexcept -> view_type {
						return self::_strip_end(this->_view(), chars);
					}

					/* @DOC Without prefix when the text starts with it, else all of it
					   (str.removeprefix). */
					[[nodiscard]] constexpr auto strip_prefix(const view_type prefix) const noexcept -> view_type {
						return this->starts_with(prefix) ? this->without_prefix(prefix.size()) : this->_view();
					}

					/* @DOC str.removesuffix. */
					[[nodiscard]] constexpr auto strip_suffix(const view_type suffix) const noexcept -> view_type {
						return this->ends_with(suffix) ? this->without_suffix(suffix.size()) : this->_view();
					}


					// -- searching ------------------------------------------

					/* @DOC Index of the first unit c, or of the first occurrence of other (0 when other
					   is empty), or nothing: str.find. */
					[[nodiscard]] constexpr auto find(const C c) const noexcept -> ca::optional<ca::usz> {

						const view_type text = this->_view();
						const C* const hit = ca::find_unit(text.data(), c, text.size());
						if (hit == nullptr)
							return ca::nullopt;
						return static_cast<ca::usz>(hit - text.data());
					}

					[[nodiscard]] constexpr auto find(const view_type other) const noexcept -> ca::optional<ca::usz> {

						const view_type text = this->_view();
						const C* const hit = self::_search(text.data(), text.data() + text.size(), other);
						if (hit == nullptr)
							return ca::nullopt;
						return static_cast<ca::usz>(hit - text.data());
					}

					/* @DOC Index of the last unit c, or of the last occurrence of other (size() when
					   other is empty), or nothing: str.rfind. */
					[[nodiscard]] constexpr auto find_last(const C c) const noexcept -> ca::optional<ca::usz> {

						const view_type text = this->_view();
						for (ca::usz i = text.size(); i != 0U; --i) {
							if (text.data()[i - 1U] == c)
								return i - 1U;
						}
						return ca::nullopt;
					}

					[[nodiscard]] constexpr auto find_last(const view_type other) const noexcept -> ca::optional<ca::usz> {

						const view_type text = this->_view();
						if (other.size() > text.size())
							return ca::nullopt;
						for (ca::usz at = text.size() - other.size() + 1U; at != 0U; --at) {
							if (ca::equal_units(text.data() + at - 1U, other.data(), other.size()))
								return at - 1U;
						}
						return ca::nullopt;
					}

					[[nodiscard]] constexpr auto contains(const C c) const noexcept -> bool {
						return this->find(c).has_value();
					}

					/* @DOC other in text. */
					[[nodiscard]] constexpr auto contains(const view_type other) const noexcept -> bool {
						return this->find(other).has_value();
					}

					/* @DOC Occurrences of unit c, or non-overlapping occurrences of other: str.count.
					   An empty other counts size() + 1. */
					[[nodiscard]] constexpr auto count(const C c) const noexcept -> ca::usz {

						const view_type text = this->_view();
						ca::usz found = 0U;
						for (const C unit : text)
							found += unit == c ? 1U : 0U;
						return found;
					}

					[[nodiscard]] constexpr auto count(const view_type other) const noexcept -> ca::usz {

						const view_type text = this->_view();
						if (other.empty())
							return text.size() + 1U;
						const C* at = text.data();
						const C* const end = text.data() + text.size();
						ca::usz found = 0U;
						for (;;) {
							const C* const hit = self::_search(at, end, other);
							if (hit == nullptr)
								return found;
							++found;
							at = hit + other.size();
						}
					}

					[[nodiscard]] constexpr auto starts_with(const view_type other) const noexcept -> bool {

						const view_type text = this->_view();
						return other.size() <= text.size() && ca::equal_units(text.data(), other.data(), other.size());
					}

					[[nodiscard]] constexpr auto ends_with(const view_type other) const noexcept -> bool {

						const view_type text = this->_view();
						return other.size() <= text.size() && ca::equal_units(text.data() + text.size() - other.size(), other.data(), other.size());
					}

					/* @DOC Lexicographic order by unsigned code unit: negative, 0 or positive. For UTF-8
					   and UTF-32 that is code point order. */
					[[nodiscard]] constexpr auto compare(const view_type other) const noexcept -> int {

						const view_type text = this->_view();
						const ca::usz common = text.size() < other.size() ? text.size() : other.size();
						const int order = ca::compare_units(text.data(), other.data(), common);
						if (order != 0)
							return order;
						return text.size() < other.size() ? -1 : text.size() > other.size() ? 1 : 0;
					}


					// -- splitting ------------------------------------------

					/* @DOC The text around the first separator: str.partition. */
					[[nodiscard]] constexpr auto partition(const view_type separator) const noexcept -> ca::sv::partitioned<C> {

						const view_type text = this->_view();
						const ca::optional<ca::usz> at = this->find(separator);
						if (!at || separator.empty())
							return ca::sv::partitioned<C>{text, view_type{}, view_type{}};
						return ca::sv::partitioned<C>{text.prefix(*at), text.slice(*at, separator.size()), text.without_prefix(*at + separator.size())};
					}

					/* @DOC The text around the last separator: str.rpartition. */
					[[nodiscard]] constexpr auto partition_last(const view_type separator) const noexcept -> ca::sv::partitioned<C> {

						const view_type text = this->_view();
						const ca::optional<ca::usz> at = this->find_last(separator);
						if (!at || separator.empty())
							return ca::sv::partitioned<C>{view_type{}, view_type{}, text};
						return ca::sv::partitioned<C>{text.prefix(*at), text.slice(*at, separator.size()), text.without_prefix(*at + separator.size())};
					}

					/* @DOC Pieces between runs of white space, none empty (str.split()), or between
					   separators, empty ones kept (str.split(sep), at most max cuts). Lazy: see
					   sv::splits. */
					[[nodiscard]] constexpr auto split(void) const noexcept -> ca::sv::splits<C> {

						const view_type text = this->_view();
						return ca::sv::splits<C>{text.data(), text.size(), nullptr, 0U, ~ca::usz{0U}, ca::sv::splits<C>::mode::white_space};
					}

					[[nodiscard]] constexpr auto split(const view_type separator) const noexcept -> ca::sv::splits<C> {
						return this->split(separator, ~ca::usz{0U});
					}

					[[nodiscard]] constexpr auto split(const view_type separator, const ca::usz max) const noexcept -> ca::sv::splits<C> {

						const view_type text = this->_view();
						return ca::sv::splits<C>{text.data(), text.size(), separator.data(), separator.size(), max, ca::sv::splits<C>::mode::separator};
					}

					/* @DOC Lines, boundaries excluded or kept (str.splitlines): \n \r \r\n \v \f \x1c
					   \x1d \x1e, and U+0085 U+2028 U+2029 once the tables cover them. */
					[[nodiscard]] constexpr auto split_lines(void) const noexcept -> ca::sv::splits<C> {
						return this->split_lines(false);
					}

					[[nodiscard]] constexpr auto split_lines(const bool keep_ends) const noexcept -> ca::sv::splits<C> {

						const view_type text = this->_view();
						return ca::sv::splits<C>{text.data(), text.size(), nullptr, 0U, ~ca::usz{0U}, keep_ends ? ca::sv::splits<C>::mode::lines_kept : ca::sv::splits<C>::mode::lines};
					}


					// -- classes --------------------------------------------

					/* @DOC Not empty, every code point a letter: str.isalpha. */
					[[nodiscard]] constexpr auto is_alpha(void) const noexcept -> bool {
						return this->_all(ca::unicode::impl::char_record::ALPHA);
					}

					/* @DOC str.isalnum. */
					[[nodiscard]] constexpr auto is_alnum(void) const noexcept -> bool {
						return this->_all(ca::unicode::impl::char_record::ALPHA | ca::unicode::impl::char_record::DECIMAL | ca::unicode::impl::char_record::DIGIT | ca::unicode::impl::char_record::NUMERIC);
					}

					/* @DOC str.isdecimal. */
					[[nodiscard]] constexpr auto is_decimal(void) const noexcept -> bool {
						return this->_all(ca::unicode::impl::char_record::DECIMAL);
					}

					/* @DOC str.isdigit. */
					[[nodiscard]] constexpr auto is_digit(void) const noexcept -> bool {
						return this->_all(ca::unicode::impl::char_record::DIGIT);
					}

					/* @DOC str.isnumeric. */
					[[nodiscard]] constexpr auto is_numeric(void) const noexcept -> bool {
						return this->_all(ca::unicode::impl::char_record::NUMERIC);
					}

					/* @DOC str.isspace. */
					[[nodiscard]] constexpr auto is_space(void) const noexcept -> bool {
						return this->_all(ca::unicode::impl::char_record::SPACE);
					}

					/* @DOC At least one cased code point, none lowercase or titlecase: str.isupper. */
					[[nodiscard]] constexpr auto is_upper(void) const noexcept -> bool {
						return this->_cased_only(true);
					}

					/* @DOC At least one cased code point, none uppercase or titlecase: str.islower. */
					[[nodiscard]] constexpr auto is_lower(void) const noexcept -> bool {
						return this->_cased_only(false);
					}

					/* @DOC Upper and titlecase only after uncased code points, lowercase only after
					   cased ones, at least one cased: str.istitle. */
					[[nodiscard]] constexpr auto is_title(void) const noexcept -> bool {

						const view_type text = this->_view();
						bool cased = false;
						bool previous_cased = false;
						for (const C* at = text.data(), * const end = at + text.size(); at != end;) {
							const ca::unicode::decoded point = ca::sv::impl::point_at(at, end);
							const ca::unicode::properties p = ca::unicode::properties_of(point.code_point());
							at += point.length();
							if (p.is_upper() || p.is_title()) {
								if (previous_cased)
									return false;
								previous_cased = cased = true;
							}
							else if (p.is_lower()) {
								if (!previous_cased)
									return false;
								previous_cased = cased = true;
							}
							else
								previous_cased = false;
						}
						return cased;
					}

					/* @DOC Every code point printable, true when empty: str.isprintable. */
					[[nodiscard]] constexpr auto is_printable(void) const noexcept -> bool {
						return this->_view().empty() || this->_is_printable_nonempty();
					}

					/* @DOC Every code unit below 0x80, true when empty: str.isascii. */
					[[nodiscard]] constexpr auto is_ascii(void) const noexcept -> bool {

						for (const C unit : this->_view()) {
							if (static_cast<ca::uint_of<sizeof(C) * 8U>>(unit) >= 0x80U)
								return false;
						}
						return true;
					}

					/* @DOC An identifier start then identifier continues: str.isidentifier. */
					[[nodiscard]] constexpr auto is_identifier(void) const noexcept -> bool {

						const view_type text = this->_view();
						if (text.empty())
							return false;
						const C* const end = text.data() + text.size();
						const ca::unicode::decoded first = ca::sv::impl::point_at(text.data(), end);
						if (!first.valid() || !ca::unicode::properties_of(first.code_point()).is_identifier_start())
							return false;
						for (const C* at = text.data() + first.length(); at != end;) {
							const ca::unicode::decoded point = ca::sv::impl::point_at(at, end);
							if (!point.valid() || !ca::unicode::properties_of(point.code_point()).is_identifier_continue())
								return false;
							at += point.length();
						}
						return true;
					}


					// -- code points ----------------------------------------

					/* @DOC The code points, for range-for (U+FFFD for invalid sequences). */
					[[nodiscard]] constexpr auto code_points(void) const noexcept -> ca::unicode::code_points<C> {
						return ca::unicode::code_points<C>{this->_view()};
					}

					/* @DOC How many code points: Python's len(). */
					[[nodiscard]] constexpr auto code_point_count(void) const noexcept -> ca::usz {
						return ca::unicode::count_code_points(this->_view());
					}


				private:

					// -- private methods ------------------------------------

					[[nodiscard]] constexpr auto _view(void) const noexcept -> view_type {
						return static_cast<const T&>(*this).view();
					}

					[[nodiscard]] constexpr auto _is_printable_nonempty(void) const noexcept -> bool {
						return this->_all(ca::unicode::impl::char_record::PRINTABLE);
					}

					/* @DOC Not empty, and every code point has one of the flags. */
					[[nodiscard]] constexpr auto _all(const ca::u16 mask) const noexcept -> bool {

						const view_type text = this->_view();
						if (text.empty())
							return false;
						for (const C* at = text.data(), * const end = at + text.size(); at != end;) {
							const ca::u32 unit = static_cast<ca::uint_of<sizeof(C) * 8U>>(*at);
							if (unit < 0x80U) [[likely]] {
								if ((ca::unicode::impl::ascii._flags[unit] & mask) == 0U)
									return false;
								++at;
								continue;
							}
							const ca::unicode::decoded point = ca::unicode::impl::decode_at(at, end);
							if (!point.valid() || (ca::unicode::impl::flags_of(point.code_point()) & mask) == 0U)
								return false;
							at += point.length();
						}
						return true;
					}

					[[nodiscard]] constexpr auto _cased_only(const bool upper) const noexcept -> bool {

						const view_type text = this->_view();
						bool cased = false;
						for (const C* at = text.data(), * const end = at + text.size(); at != end;) {
							const ca::unicode::decoded point = ca::sv::impl::point_at(at, end);
							const ca::unicode::properties p = ca::unicode::properties_of(point.code_point());
							at += point.length();
							if ((upper ? p.is_lower() : p.is_upper()) || p.is_title())
								return false;
							if (upper ? p.is_upper() : p.is_lower())
								cased = true;
						}
						return cased;
					}

					[[nodiscard]] static constexpr auto _search(const C* const begin, const C* const end, const view_type other) noexcept -> const C* {

						if (other.empty())
							return begin;
						if (other.size() > static_cast<ca::usz>(end - begin))
							return nullptr;
						const C first = other.data()[0];
						const C final = other.data()[other.size() - 1U];
						const ca::usz tail = other.size() - 1U;
						const C* at = begin;
						const C* const last = end - other.size();
						while (at <= last) {
							const C* const hit = ca::find_unit(at, first, static_cast<ca::usz>(last - at) + 1U);
							if (hit == nullptr)
								return nullptr;
							if (hit[tail] == final && ca::equal_units(hit + 1, other.data() + 1, tail))
								return hit;
							at = hit + 1;
						}
						return nullptr;
					}

					[[nodiscard]] static constexpr auto _in(const view_type chars, const char32_t point) noexcept -> bool {

						for (const C* at = chars.data(), * const end = at + chars.size(); at != end;) {
							const ca::unicode::decoded candidate = ca::sv::impl::point_at(at, end);
							if (candidate.code_point() == point)
								return true;
							at += candidate.length();
						}
						return false;
					}

					[[nodiscard]] static constexpr auto _strip_start(const view_type text) noexcept -> view_type {

						const C* at = text.data();
						const C* const end = text.data() + text.size();
						while (at != end) {
							const ca::u8 space = ca::sv::impl::space_at(at, end);
							if (space == 0U)
								break;
							at += space;
						}
						return view_type{at, static_cast<ca::usz>(end - at)};
					}

					[[nodiscard]] static constexpr auto _strip_end(const view_type text) noexcept -> view_type {

						const C* const begin = text.data();
						const C* end = text.data() + text.size();
						while (end != begin) {
							const ca::u8 space = ca::sv::impl::space_before(begin, end);
							if (space == 0U)
								break;
							end -= space;
						}
						return view_type{begin, static_cast<ca::usz>(end - begin)};
					}

					[[nodiscard]] static constexpr auto _strip_start(const view_type text, const view_type chars) noexcept -> view_type {

						const C* at = text.data();
						const C* const end = text.data() + text.size();
						while (at != end) {
							const ca::unicode::decoded point = ca::sv::impl::point_at(at, end);
							if (!point.valid() || !self::_in(chars, point.code_point()))
								break;
							at += point.length();
						}
						return view_type{at, static_cast<ca::usz>(end - at)};
					}

					[[nodiscard]] static constexpr auto _strip_end(const view_type text, const view_type chars) noexcept -> view_type {

						const C* const begin = text.data();
						const C* end = text.data() + text.size();
						while (end != begin) {
							const ca::unicode::decoded point = ca::sv::impl::point_before(begin, end);
							if (!point.valid() || !self::_in(chars, point.code_point()))
								break;
							end -= point.length();
						}
						return view_type{begin, static_cast<ca::usz>(end - begin)};
					}

			}; // class methods

		} // namespace impl

	} // namespace sv

} // namespace ca

#endif // ca_vocabulary_sv_impl_methods_hpp
