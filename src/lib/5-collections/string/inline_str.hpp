/* @DOC Performance of ca::inline_str<N, C>.

   Layout: the size in the smallest unsigned type holding N (1 byte up to 255, 2 up to 65535),
   then N + 1 units: no pointer, no allocation, no indirection. inline_str<15> is 17 bytes.
   Trivially copyable: copied and moved as bytes, usable in constant expressions.

   | Operation | Time | Notes |
   |---|---|---|
   | size, data, view, operator[] | O(1) | |
   | push | O(1) | one compare against N |
   | append, assign | O(n) | one memcpy outside constant evaluation |
   | insert(i, text), remove(i, n) | O(size - i) | one memmove |

   Nothing grows: what does not fit fails with errc::no_space and changes nothing. */

#ifndef ca_collections_string_inline_str_hpp
#define ca_collections_string_inline_str_hpp

#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/move_bytes.hpp"
#include "1-primitives/text/is_character.hpp"
#include "1-primitives/traits/conditional.hpp"
#include "1-primitives/traits/is_constant_evaluated.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/basic.hpp"
#include "2-vocabulary/sv/impl/methods.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "2-vocabulary/unicode/encode.hpp"
#include "2-vocabulary/unicode/max_units.hpp"
#include "3-system/error/errc.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- INLINE STR ---------------------------------------------------------

	/* @DOC String of at most N code units C stored in the object itself, NUL-terminated: no
	   allocation, trivially copyable, constexpr. Operations that would pass N fail with
	   errc::no_space and change nothing. Converts to sv::basic<C>; has the read-only Python-like
	   methods of the views (sv::impl::methods). operator[] traps out
	   of range; get allows the doubt. */
	template <ca::usz N, ca::is_character C = char>
	class inline_str final : public ca::sv::impl::methods<ca::inline_str<N, C>, C> {


		private:

			// -- private types ----------------------------------------------

			using self = ca::inline_str<N, C>;

			using size_word = ca::conditional<(N <= 0xFFU), ca::u8, ca::conditional<(N <= 0xFFFFU), ca::u16, ca::usz>>;


		public:

			// -- public types -----------------------------------------------

			using value_type      = C;
			using size_type       = ca::usz;
			using reference       = C&;
			using const_reference = const C&;
			using iterator        = C*;
			using const_iterator  = const C*;
			using view_type       = ca::sv::basic<C>;


			// -- public constants -------------------------------------------

			/* @DOC Most code units held, NUL excluded. */
			static constexpr ca::usz CAPACITY = N;


		private:

			// -- private members --------------------------------------------

			size_word _size;
			C         _data[N + 1U];


		public:

			// -- public lifecycle -------------------------------------------

			constexpr inline_str(void) noexcept
			: _size{0U}, _data{} {
			}

			/* @DOC A copy of text; errc::no_space when it is longer than N. */
			[[nodiscard]] static constexpr auto make(const view_type text) noexcept -> ca::result<self> {

				self out{};
				auto assigned = out.assign(text);
				if (!assigned)
					return assigned.propagate();
				return out;
			}


			// -- iterators --------------------------------------------------

			[[nodiscard]] constexpr auto begin(void) noexcept -> C* {
				return _data;
			}

			[[nodiscard]] constexpr auto begin(void) const noexcept -> const C* {
				return _data;
			}

			[[nodiscard]] constexpr auto end(void) noexcept -> C* {
				return _data + _size;
			}

			[[nodiscard]] constexpr auto end(void) const noexcept -> const C* {
				return _data + _size;
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
				return _size;
			}

			[[nodiscard]] static constexpr auto capacity(void) noexcept -> ca::usz {
				return N;
			}

			[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
				return _size == 0U;
			}

			[[nodiscard]] constexpr auto full(void) const noexcept -> bool {
				return _size == N;
			}

			/* @DOC The units, followed by a NUL. */
			[[nodiscard]] constexpr auto data(void) noexcept -> C* {
				return _data;
			}

			[[nodiscard]] constexpr auto data(void) const noexcept -> const C* {
				return _data;
			}

			[[nodiscard]] constexpr auto c_str(void) const noexcept -> const C* {
				return _data;
			}

			[[nodiscard]] constexpr auto view(void) const noexcept -> view_type {
				return view_type{_data, _size};
			}

			/* @DOC view(). */
			constexpr operator view_type(void) const noexcept {
				return view_type{_data, _size};
			}


			// -- element access ---------------------------------------------

			/* @DOC Unit at index. Traps when index >= size(), in every build. */
			[[nodiscard]] constexpr auto operator[](const ca::usz index) noexcept -> C& {

				if (index >= _size) [[unlikely]]
					ca::impl::index_out_of_bounds();
				return _data[index];
			}

			[[nodiscard]] constexpr auto operator[](const ca::usz index) const noexcept -> const C& {

				if (index >= _size) [[unlikely]]
					ca::impl::index_out_of_bounds();
				return _data[index];
			}

			/* @DOC Unit at index, or nothing when index >= size(). */
			[[nodiscard]] constexpr auto get(const ca::usz index) noexcept -> ca::optional<C&> {

				if (index >= _size)
					return ca::nullopt;
				return ca::optional<C&>{_data[index]};
			}

			[[nodiscard]] constexpr auto get(const ca::usz index) const noexcept -> ca::optional<const C&> {

				if (index >= _size)
					return ca::nullopt;
				return ca::optional<const C&>{_data[index]};
			}


			// -- modifiers --------------------------------------------------

			/* @DOC Appends c; errc::no_space when full. */
			[[nodiscard]] constexpr auto push(const C c) noexcept -> ca::result<void> {

				if (_size == N) [[unlikely]]
					return ca::failure{ca::errc::no_space};
				_data[_size] = c;
				this->_set_size(_size + 1U);
				return ca::result<void>{};
			}

			/* @DOC Appends all of text, which may be a view of this string, or nothing
			   (errc::no_space). */
			[[nodiscard]] constexpr auto append(const view_type text) noexcept -> ca::result<void> {

				const ca::usz count = text.size();
				if (count > N - _size) [[unlikely]]
					return ca::failure{ca::errc::no_space};
				self::_copy(_data + _size, text.data(), count);
				this->_set_size(_size + count);
				return ca::result<void>{};
			}

			/* @DOC Appends point encoded in C's encoding (U+FFFD when not a scalar value). */
			[[nodiscard]] constexpr auto append_code_point(const char32_t point) noexcept -> ca::result<void> {

				C units[ca::unicode::max_units<C>]{};
				return this->append(view_type{units, ca::unicode::encode(point, units)});
			}

			/* @DOC Replaces the text with text, which may be a view of this string, or changes
			   nothing (errc::no_space). */
			[[nodiscard]] constexpr auto assign(const view_type text) noexcept -> ca::result<void> {

				const ca::usz count = text.size();
				if (count > N) [[unlikely]]
					return ca::failure{ca::errc::no_space};
				if (ca::is_constant_evaluated()) {
					C staged[N + 1U]{};
					self::_copy(staged, text.data(), count);
					self::_copy(_data, staged, count);
				}
				else
					ca::move_bytes(_data, text.data(), count * sizeof(C));
				this->_set_size(count);
				return ca::result<void>{};
			}

			/* @DOC Inserts text at index, which may be a view of this string, shifting the rest, or
			   changes nothing (errc::no_space). Traps when index > size(), in every build. */
			[[nodiscard]] constexpr auto insert(const ca::usz index, const view_type text) noexcept -> ca::result<void> {

				if (index > _size) [[unlikely]]
					ca::impl::index_out_of_bounds();
				const ca::usz count = text.size();
				if (count > N - _size) [[unlikely]]
					return ca::failure{ca::errc::no_space};
				if (ca::is_constant_evaluated()) {
					C staged[N + 1U]{};
					self::_copy(staged, text.data(), count);
					self::_move(_data + index + count, _data + index, _size - index);
					self::_copy(_data + index, staged, count);
				}
				else {
					const ca::optional<ca::usz> inside = this->_offset_of(text.data());
					ca::move_bytes(_data + index + count, _data + index, (_size - index) * sizeof(C));
					if (inside) {
						for (ca::usz i = 0U; i < count; ++i) {
							const ca::usz from = *inside + i;
							_data[index + i] = _data[from >= index ? from + count : from];
						}
					}
					else
						ca::copy_bytes(_data + index, text.data(), count * sizeof(C));
				}
				this->_set_size(_size + count);
				return ca::result<void>{};
			}

			/* @DOC Removes up to count units from index, both clamped, shifting the rest. */
			constexpr auto remove(const ca::usz index, const ca::usz count) noexcept -> void {

				if (index >= _size)
					return;
				const ca::usz removed = count < _size - index ? count : _size - index;
				self::_move(_data + index, _data + index + removed, _size - index - removed);
				this->_set_size(_size - removed);
			}

			/* @DOC Sets size() to count, new units set to fill (NUL by default), or changes nothing
			   (errc::no_space). */
			[[nodiscard]] constexpr auto resize(const ca::usz count) noexcept -> ca::result<void> {
				return this->resize(count, C{});
			}

			[[nodiscard]] constexpr auto resize(const ca::usz count, const C fill) noexcept -> ca::result<void> {

				if (count > N) [[unlikely]]
					return ca::failure{ca::errc::no_space};
				for (ca::usz i = _size; i < count; ++i)
					_data[i] = fill;
				this->_set_size(count);
				return ca::result<void>{};
			}

			/* @DOC Sets size() to count without touching the units, for code that wrote them into
			   data() up to N. Writes the NUL. Traps when count > N, in every build. */
			constexpr auto set_size(const ca::usz count) noexcept -> void {

				if (count > N) [[unlikely]]
					ca::impl::index_out_of_bounds();
				this->_set_size(count);
			}

			/* @DOC Removes and returns the last unit, or nothing when empty. */
			[[nodiscard]] constexpr auto pop(void) noexcept -> ca::optional<C> {

				if (_size == 0U)
					return ca::nullopt;
				const C last = _data[_size - 1U];
				this->_set_size(_size - 1U);
				return last;
			}

			constexpr auto clear(void) noexcept -> void {
				this->_set_size(0U);
			}


			// -- methods ----------------------------------------------------

			[[nodiscard]] constexpr auto operator==(const self& other) const noexcept -> bool {
				return this->view() == other.view();
			}

			[[nodiscard]] constexpr auto operator==(const view_type other) const noexcept -> bool {
				return this->view() == other;
			}


		private:

			// -- private methods --------------------------------------------

			constexpr auto _set_size(const ca::usz size) noexcept -> void {

				      _size = static_cast<size_word>(size);
				_data[size] = C{};
			}

			[[nodiscard]] auto _offset_of(const C* const at) const noexcept -> ca::optional<ca::usz> {

				if (at >= _data && at <= _data + N)
					return static_cast<ca::usz>(at - _data);
				return ca::nullopt;
			}

			static constexpr auto _copy(C* const to, const C* const from, const ca::usz count) noexcept -> void {

				if (!ca::is_constant_evaluated()) {
					ca::copy_bytes(to, from, count * sizeof(C));
					return;
				}
				for (ca::usz i = 0U; i < count; ++i)
					to[i] = from[i];
			}

			static constexpr auto _move(C* const to, const C* const from, const ca::usz count) noexcept -> void {

				if (!ca::is_constant_evaluated()) {
					ca::move_bytes(to, from, count * sizeof(C));
					return;
				}
				if (to < from) {
					for (ca::usz i = 0U; i < count; ++i)
						to[i] = from[i];
				}
				else {
					for (ca::usz i = count; i != 0U; --i)
						to[i - 1U] = from[i - 1U];
				}
			}

	}; // class inline_str

} // namespace ca

#endif // ca_collections_string_inline_str_hpp
