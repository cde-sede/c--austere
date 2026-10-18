#ifndef ca_vocabulary_borrowed_slice_hpp
#define ca_vocabulary_borrowed_slice_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/equal_units.hpp"
#include "1-primitives/memory/find_unit.hpp"
#include "1-primitives/text/is_character.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cv.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/utility/declval.hpp"
#include "1-primitives/utility/initializer_list.hpp"
#include "1-primitives/utility/swap.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/impl/view_rules.hpp"
#include "2-vocabulary/is_contiguous.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BORROWED SLICE -----------------------------------------------------

	template <typename T>
	class borrowed_slice;


	namespace impl {

		/* @DOC The sequence methods, written once for every type D whose view() gives a
		   borrowed_slice: borrowed_slice<T>, ca::slice<T, A>, dynamic_array<T, A>, ca::array<T, N>,
		   inline_array<T, N>. T is the element
		   type a mutable D hands out, V the one a const D does (const T for owners, T for
		   borrowed_slice). Cutting returns views and never allocates; searching works with ==
		   (code units: memchr and memcmp), compare with <. */
		template <typename D, typename T, typename V> // final: ignore
		class slice_methods {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::slice_methods<D, T, V>;

				using view_type       = ca::borrowed_slice<T>;
				using const_view_type = ca::borrowed_slice<V>;
				using value_type      = ca::remove_cv<T>;
				using other_type      = ca::borrowed_slice<const ca::remove_cv<T>>;


			public:

				// -- cutting ------------------------------------------------

				/* @DOC First count elements, or all of them. */
				[[nodiscard]] constexpr auto prefix(const ca::usz count) noexcept -> view_type {
					return self::_prefix(this->_mutable_view(), count);
				}

				[[nodiscard]] constexpr auto prefix(const ca::usz count) const noexcept -> const_view_type {
					return self::_prefix(this->_view(), count);
				}

				/* @DOC Last count elements, or all of them. */
				[[nodiscard]] constexpr auto suffix(const ca::usz count) noexcept -> view_type {
					return self::_suffix(this->_mutable_view(), count);
				}

				[[nodiscard]] constexpr auto suffix(const ca::usz count) const noexcept -> const_view_type {
					return self::_suffix(this->_view(), count);
				}

				/* @DOC All but the first count elements; empty when count >= size(). */
				[[nodiscard]] constexpr auto without_prefix(const ca::usz count) noexcept -> view_type {
					return self::_without_prefix(this->_mutable_view(), count);
				}

				[[nodiscard]] constexpr auto without_prefix(const ca::usz count) const noexcept -> const_view_type {
					return self::_without_prefix(this->_view(), count);
				}

				/* @DOC All but the last count elements; empty when count >= size(). */
				[[nodiscard]] constexpr auto without_suffix(const ca::usz count) noexcept -> view_type {
					return self::_without_suffix(this->_mutable_view(), count);
				}

				[[nodiscard]] constexpr auto without_suffix(const ca::usz count) const noexcept -> const_view_type {
					return self::_without_suffix(this->_view(), count);
				}

				/* @DOC Up to count elements from offset, both clamped: s[offset:offset + count]. */
				[[nodiscard]] constexpr auto sub(const ca::usz offset, const ca::usz count) noexcept -> view_type {
					return self::_prefix(self::_without_prefix(this->_mutable_view(), offset), count);
				}

				[[nodiscard]] constexpr auto sub(const ca::usz offset, const ca::usz count) const noexcept -> const_view_type {
					return self::_prefix(self::_without_prefix(this->_view(), offset), count);
				}


				// -- element access -----------------------------------------

				/* @DOC First element, or nothing when empty. */
				[[nodiscard]] constexpr auto first(void) noexcept -> ca::optional<T&> {
					return this->_mutable_view().get(0U);
				}

				[[nodiscard]] constexpr auto first(void) const noexcept -> ca::optional<V&> {
					return this->_view().get(0U);
				}

				/* @DOC Last element, or nothing when empty. */
				[[nodiscard]] constexpr auto last(void) noexcept -> ca::optional<T&> {

					const view_type all = this->_mutable_view();
					return all.get(all.size() - 1U);
				}

				[[nodiscard]] constexpr auto last(void) const noexcept -> ca::optional<V&> {

					const const_view_type all = this->_view();
					return all.get(all.size() - 1U);
				}


				// -- searching ----------------------------------------------

				/* @DOC Index of the first element equal to value, or of the first occurrence of other
				   (0 when other is empty), or nothing. */
				[[nodiscard]] constexpr auto find(const value_type& value) const noexcept -> ca::optional<ca::usz> {

					const const_view_type all = this->_view();
					if constexpr (ca::is_character<value_type>) {
						const value_type* const hit = ca::find_unit<value_type>(all.data(), value, all.size());
						if (hit == nullptr)
							return ca::nullopt;
						return static_cast<ca::usz>(hit - all.data());
					}
					else {
						for (ca::usz i = 0U; i < all.size(); ++i) {
							if (all.data()[i] == value)
								return i;
						}
						return ca::nullopt;
					}
				}

				[[nodiscard]] constexpr auto find(const other_type other) const noexcept -> ca::optional<ca::usz> {

					const const_view_type all = this->_view();
					if (other.size() > all.size())
						return ca::nullopt;
					for (ca::usz at = 0U; at <= all.size() - other.size(); ++at) {
						if (self::_equal(all.data() + at, other.data(), other.size()))
							return at;
					}
					return ca::nullopt;
				}

				/* @DOC Index of the last element equal to value, or of the last occurrence of other
				   (size() when other is empty), or nothing. */
				[[nodiscard]] constexpr auto find_last(const value_type& value) const noexcept -> ca::optional<ca::usz> {

					const const_view_type all = this->_view();
					for (ca::usz i = all.size(); i != 0U; --i) {
						if (all.data()[i - 1U] == value)
							return i - 1U;
					}
					return ca::nullopt;
				}

				[[nodiscard]] constexpr auto find_last(const other_type other) const noexcept -> ca::optional<ca::usz> {

					const const_view_type all = this->_view();
					if (other.size() > all.size())
						return ca::nullopt;
					for (ca::usz at = all.size() - other.size() + 1U; at != 0U; --at) {
						if (self::_equal(all.data() + at - 1U, other.data(), other.size()))
							return at - 1U;
					}
					return ca::nullopt;
				}

				/* @DOC Index of the first element for which test(element) holds, or nothing. */
				template <typename F>
				[[nodiscard]] constexpr auto find_if(F&& test) const noexcept -> ca::optional<ca::usz> {

					const const_view_type all = this->_view();
					for (ca::usz i = 0U; i < all.size(); ++i) {
						if (test(all.data()[i]))
							return i;
					}
					return ca::nullopt;
				}

				[[nodiscard]] constexpr auto contains(const value_type& value) const noexcept -> bool {
					return this->find(value).has_value();
				}

				/* @DOC other occurs in it. */
				[[nodiscard]] constexpr auto contains(const other_type other) const noexcept -> bool {
					return this->find(other).has_value();
				}

				/* @DOC Elements equal to value, or non-overlapping occurrences of other. An empty
				   other counts size() + 1. */
				[[nodiscard]] constexpr auto count(const value_type& value) const noexcept -> ca::usz {

					ca::usz found = 0U;
					for (const value_type& element : this->_view())
						found += element == value ? 1U : 0U;
					return found;
				}

				[[nodiscard]] constexpr auto count(const other_type other) const noexcept -> ca::usz {

					const const_view_type all = this->_view();
					if (other.empty())
						return all.size() + 1U;
					ca::usz found = 0U;
					for (ca::usz at = 0U; at + other.size() <= all.size();) {
						if (self::_equal(all.data() + at, other.data(), other.size())) {
							++found;
							at += other.size();
						}
						else
							++at;
					}
					return found;
				}

				[[nodiscard]] constexpr auto starts_with(const other_type other) const noexcept -> bool {

					const const_view_type all = this->_view();
					return other.size() <= all.size() && self::_equal(all.data(), other.data(), other.size());
				}

				[[nodiscard]] constexpr auto ends_with(const other_type other) const noexcept -> bool {

					const const_view_type all = this->_view();
					return other.size() <= all.size() && self::_equal(all.data() + all.size() - other.size(), other.data(), other.size());
				}

				/* @DOC Lexicographic order by <: negative, 0 or positive. */
				[[nodiscard]] constexpr auto compare(const other_type other) const noexcept -> int {

					const const_view_type all = this->_view();
					const ca::usz common = all.size() < other.size() ? all.size() : other.size();
					for (ca::usz i = 0U; i < common; ++i) {
						if (all.data()[i] < other.data()[i])
							return -1;
						if (other.data()[i] < all.data()[i])
							return 1;
					}
					return all.size() < other.size() ? -1 : all.size() > other.size() ? 1 : 0;
				}


				// -- modifiers ----------------------------------------------

				/* @DOC Assigns value to every element. */
				constexpr auto fill(const value_type& value) noexcept -> void requires (!ca::is_const<T>) {
					for (T& element : this->_mutable_view())
						element = value;
				}

				/* @DOC Reverses the order of the elements in place. */
				constexpr auto reverse(void) noexcept -> void requires (!ca::is_const<T>) {

					const view_type all = this->_mutable_view();
					if (all.empty())
						return;
					for (T* low = all.data(), * high = all.data() + all.size() - 1U; low < high; ++low, --high)
						ca::swap(*low, *high);
				}


			private:

				// -- private methods ----------------------------------------

				[[nodiscard]] constexpr auto _view(void) const noexcept -> const_view_type {
					return static_cast<const D&>(*this).view();
				}

				[[nodiscard]] constexpr auto _mutable_view(void) noexcept -> view_type {
					return static_cast<D&>(*this).view();
				}

				template <typename U>
				[[nodiscard]] static constexpr auto _prefix(const ca::borrowed_slice<U> all, const ca::usz count) noexcept -> ca::borrowed_slice<U> {
					return ca::borrowed_slice<U>{all.data(), count < all.size() ? count : all.size()};
				}

				template <typename U>
				[[nodiscard]] static constexpr auto _suffix(const ca::borrowed_slice<U> all, const ca::usz count) noexcept -> ca::borrowed_slice<U> {

					const ca::usz kept = count < all.size() ? count : all.size();
					return ca::borrowed_slice<U>{all.data() + all.size() - kept, kept};
				}

				template <typename U>
				[[nodiscard]] static constexpr auto _without_prefix(const ca::borrowed_slice<U> all, const ca::usz count) noexcept -> ca::borrowed_slice<U> {

					const ca::usz dropped = count < all.size() ? count : all.size();
					return ca::borrowed_slice<U>{all.data() + dropped, all.size() - dropped};
				}

				template <typename U>
				[[nodiscard]] static constexpr auto _without_suffix(const ca::borrowed_slice<U> all, const ca::usz count) noexcept -> ca::borrowed_slice<U> {
					return ca::borrowed_slice<U>{all.data(), count < all.size() ? all.size() - count : 0U};
				}

				[[nodiscard]] static constexpr auto _equal(const value_type* const left, const value_type* const right, const ca::usz count) noexcept -> bool {
					if constexpr (ca::is_character<value_type>)
						return ca::equal_units(left, right, count);
					else {
						for (ca::usz i = 0U; i < count; ++i) {
							if (!(left[i] == right[i]))
								return false;
						}
						return true;
					}
				}

		}; // class slice_methods


		template <typename T>
		inline constexpr bool is_borrowed_slice = false;

		template <typename T>
		inline constexpr bool is_borrowed_slice<ca::borrowed_slice<T>> = true;

		/* @DOC S has the slice methods: view() gives a borrowed_slice. */
		template <typename S>
		concept has_slice_view = requires (const S& sequence) {
			requires ca::impl::is_borrowed_slice<decltype(sequence.view())>;
		};

		/* @DOC Same size, elements equal pairwise: any two of borrowed_slice, ca::slice,
		   dynamic_array with the same element type. */
		template <ca::impl::has_slice_view L, ca::impl::has_slice_view R>
		requires ca::is_same<typename decltype(ca::declval<const L&>().view())::value_type, typename decltype(ca::declval<const R&>().view())::value_type>
		[[nodiscard]] constexpr auto operator==(const L& left, const R& right) noexcept -> bool {

			const auto all = left.view();
			const auto other = right.view();
			return all.size() == other.size() && all.starts_with(other);
		}

	} // namespace impl


	/* @DOC Sequence of size() T it does not own, for element work: cut, search, compare, fill,
	   reverse (ca::impl::slice_methods, shared with ca::slice and dynamic_array, whose views
	   these are). Made from a pointer and a size, an array (not one of const code units: text
	   goes through ca::sv), or anything contiguous of the same element type, const added at
	   most. Two words, copied in registers. Constness is the elements', not the view's. Cutting
	   functions clamp to the bounds; operator[] traps out of range, get allows the doubt. Raw
	   memory (bytes, buffers) is span's; both convert to each other for free. */
	template <typename T>
	class borrowed_slice final : public ca::impl::slice_methods<ca::borrowed_slice<T>, T, T> {


		private:

			// -- private types ----------------------------------------------

			using self = ca::borrowed_slice<T>;


		public:

			// -- public types -----------------------------------------------

			using value_type   = ca::remove_cv<T>;
			using element_type = T;
			using size_type    = ca::usz;
			using reference    = T&;
			using pointer      = T*;
			using iterator     = T*;


			// -- public constants -------------------------------------------

			/* @DOC Refers to elements it does not own: a view made from a temporary one stays valid. */
			static constexpr bool BORROWED = true;


		private:

			// -- private members --------------------------------------------

			T* _data;
			ca::usz _size;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr borrowed_slice(void) noexcept
			: _data{nullptr}, _size{0U} {
			}

			constexpr borrowed_slice(T* const data, const ca::usz size) noexcept
			: _data{data}, _size{size} {
			}

			/* @DOC The whole array. */
			template <typename U, ca::usz N>
			requires ca::is_convertible<U (*)[], T (*)[]> && (!ca::impl::is_const_text<U>)
			constexpr borrowed_slice(U (&array)[N]) noexcept
			: _data{array}, _size{N} {
			}

			/* @DOC The elements of a braced list, for an argument: `a.extend({1, 2, 3})`. The list's
			   array dies with the full expression, so a view kept past it dangles. */
			constexpr borrowed_slice(const ca::initializer_list<ca::remove_cv<T>> list) noexcept requires ca::is_const<T>
			: _data{list.begin()}, _size{list.size()} {
			}

			/* @DOC The elements of range; a temporary one only when T is const or range is a view. */
			template <typename R>
			requires (!ca::is_same<ca::remove_cvref<R>, ca::borrowed_slice<T>>) && ca::is_contiguous<R>
				&& ca::is_convertible<ca::impl::element_of<R> (*)[], T (*)[]> && ca::impl::is_viewable<R, T>
			constexpr borrowed_slice(R&& range) noexcept
			: _data{range.data()}, _size{static_cast<ca::usz>(range.size())} {
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

			/* @DOC Itself: what the shared methods work on. */
			[[nodiscard]] constexpr auto view(void) const noexcept -> self {
				return *this;
			}

	}; // class borrowed_slice


	template <typename T>
	borrowed_slice(T*, ca::usz) -> borrowed_slice<T>; // qualify: ignore

	template <typename T, ca::usz N>
	borrowed_slice(T (&)[N]) -> borrowed_slice<T>; // qualify: ignore

	template <ca::is_contiguous R>
	borrowed_slice(R&&) -> borrowed_slice<ca::impl::element_of<R>>; // qualify: ignore

} // namespace ca

#endif // ca_vocabulary_borrowed_slice_hpp
