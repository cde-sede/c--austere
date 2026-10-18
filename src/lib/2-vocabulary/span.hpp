#ifndef ca_vocabulary_span_hpp
#define ca_vocabulary_span_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/impl/is_any_of.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cv.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/impl/view_rules.hpp"
#include "2-vocabulary/is_contiguous.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SPAN ---------------------------------------------------------------

	namespace impl {

		/* @DOC One-byte types whose memory a span of ca::byte may view. */
		template <typename U>
		concept is_byte_like = ca::impl::is_any_of<ca::remove_cv<U>, char, signed char, unsigned char, char8_t>;

		/* @DOC A span<T> may view U elements: the same type, const added at most, or the bytes of
		   byte-like U, constness kept. */
		template <typename U, typename T>
		concept is_span_compatible = ca::is_convertible<U (*)[], T (*)[]>
			|| (ca::is_same<ca::remove_cv<T>, ca::byte> && ca::impl::is_byte_like<U> && (ca::is_const<T> || !ca::is_const<U>));

	} // namespace impl


	/* @DOC Memory holding size() T it does not own: buffers, I/O, mappings. Made from a pointer and
	   a size, an array (not one of const code units: a text literal's NUL would come along; text
	   goes through ca::sv), or anything contiguous; a span of ca::byte also views char, signed
	   char and char8_t memory, so text and byte buffers pass where bytes are expected. Two words,
	   copied in registers. Constness is the elements', not the span's. Cutting functions clamp
	   to the bounds; operator[] traps out of range, get allows the doubt. Sequence work (search,
	   compare, reverse) is borrowed_slice's; both convert to each other for free. */
	template <typename T>
	class span final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::span<T>;


		public:

			// -- public types -----------------------------------------------

			using value_type   = ca::remove_cv<T>;
			using element_type = T;
			using size_type    = ca::usz;
			using reference    = T&;
			using pointer      = T*;
			using iterator     = T*;


			// -- public constants -------------------------------------------

			/* @DOC Refers to memory it does not own: a view made from a temporary span stays valid. */
			static constexpr bool BORROWED = true;


		private:

			// -- private members --------------------------------------------

			T* _data;
			ca::usz _size;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr span(void) noexcept
			: _data{nullptr}, _size{0U} {
			}

			constexpr span(T* const data, const ca::usz size) noexcept
			: _data{data}, _size{size} {
			}

			/* @DOC The whole array. */
			template <typename U, ca::usz N>
			requires ca::impl::is_span_compatible<U, T> && (!ca::impl::is_const_text<U>)
			constexpr span(U (&array)[N]) noexcept
			: _data{self::_cast(array)}, _size{N} {
			}

			/* @DOC The elements of range; a temporary one only when T is const or range is a view. */
			template <typename R>
			requires (!ca::is_same<ca::remove_cvref<R>, ca::span<T>>) && ca::is_contiguous<R>
				&& ca::impl::is_span_compatible<ca::impl::element_of<R>, T> && ca::impl::is_viewable<R, T>
			constexpr span(R&& range) noexcept
			: _data{self::_cast(range.data())}, _size{static_cast<ca::usz>(range.size())} {
			}


			// -- iterators --------------------------------------------------

			[[nodiscard]] constexpr auto begin(void) const noexcept -> T* {
				return _data;
			}

			[[nodiscard]] constexpr auto end(void) const noexcept -> T* {
				return _data + _size;
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto data(void) const noexcept -> T* {
				return _data;
			}

			/* @DOC Length in elements. */
			[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
				return _size;
			}

			/* @DOC Length in bytes. */
			[[nodiscard]] constexpr auto size_bytes(void) const noexcept -> ca::usz {
				return _size * sizeof(T);
			}

			[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
				return _size == 0U;
			}


			// -- element access ---------------------------------------------

			/* @DOC Element at index. Traps when index >= size(), in every build. */
			[[nodiscard]] constexpr auto operator[](const ca::usz index) const noexcept -> T& {

				if (index >= _size) [[unlikely]]
					ca::impl::index_out_of_bounds();
				return _data[index];
			}

			/* @DOC Element at index, or nothing when index >= size(). */
			[[nodiscard]] constexpr auto get(const ca::usz index) const noexcept -> ca::optional<T&> {

				if (index >= _size)
					return ca::nullopt;
				return ca::optional<T&>{_data[index]};
			}


			// -- methods ----------------------------------------------------

			/* @DOC First count elements, or all of them. */
			[[nodiscard]] constexpr auto prefix(const ca::usz count) const noexcept -> self {
				return self{_data, count < _size ? count : _size};
			}

			/* @DOC Last count elements, or all of them. */
			[[nodiscard]] constexpr auto suffix(const ca::usz count) const noexcept -> self {

				const ca::usz kept = count < _size ? count : _size;
				return self{_data + _size - kept, kept};
			}

			/* @DOC All but the first count elements; empty when count >= size(). */
			[[nodiscard]] constexpr auto without_prefix(const ca::usz count) const noexcept -> self {

				const ca::usz dropped = count < _size ? count : _size;
				return self{_data + dropped, _size - dropped};
			}

			/* @DOC All but the last count elements; empty when count >= size(). */
			[[nodiscard]] constexpr auto without_suffix(const ca::usz count) const noexcept -> self {
				return self{_data, count < _size ? _size - count : 0U};
			}

			/* @DOC Up to count elements from offset, both clamped. */
			[[nodiscard]] constexpr auto sub(const ca::usz offset, const ca::usz count) const noexcept -> self {
				return this->without_prefix(offset).prefix(count);
			}

			/* @DOC The same memory as bytes. */
			[[nodiscard]] auto bytes(void) const noexcept -> ca::span<const ca::byte> {
				return ca::span<const ca::byte>{reinterpret_cast<const ca::byte*>(_data), this->size_bytes()};
			}

			/* @DOC The same memory as writable bytes. */
			[[nodiscard]] auto writable_bytes(void) const noexcept -> ca::span<ca::byte> requires (!ca::is_const<T>) {
				return ca::span<ca::byte>{reinterpret_cast<ca::byte*>(_data), this->size_bytes()};
			}


		private:

			// -- private methods --------------------------------------------

			template <typename U>
			[[nodiscard]] static constexpr auto _cast(U* const data) noexcept -> T* {

				if constexpr (ca::is_convertible<U (*)[], T (*)[]>)
					return data;
				else
					return reinterpret_cast<T*>(data);
			}

	}; // class span


	template <typename T>
	span(T*, ca::usz) -> span<T>; // qualify: ignore

	template <typename T, ca::usz N>
	span(T (&)[N]) -> span<T>; // qualify: ignore

	template <ca::is_contiguous R>
	span(R&&) -> span<ca::impl::element_of<R>>; // qualify: ignore

} // namespace ca

#endif // ca_vocabulary_span_hpp
