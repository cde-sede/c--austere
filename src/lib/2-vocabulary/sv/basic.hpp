#ifndef ca_vocabulary_sv_basic_hpp
#define ca_vocabulary_sv_basic_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/equal_units.hpp"
#include "1-primitives/memory/string_length.hpp"
#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/sv/impl/forward.hpp"
#include "2-vocabulary/sv/impl/methods.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SV -----------------------------------------------------------------

	/* @DOC String views: read-only runs of code units they do not own. sv::basic<C> for any
	   code unit; sv::str (char), sv::u8 (char8_t), sv::u16, sv::u32, sv::wide (wchar_t);
	   utf8, utf16, utf32 name u8, u16, u32. */
	namespace sv {


		// -- BASIC ----------------------------------------------------------

		/* @DOC Read-only run of code units of type C it does not own; not NUL-terminated. Sizes
		   and indices count code units, not code points (ca::unicode walks those). Cutting
		   functions clamp to the bounds. operator[] traps out of range; get allows the doubt. The
		   Python-like methods (strip, split, partition, count, is_alpha, ...) come from
		   sv::impl::methods. */
		template <ca::is_character C>
		class basic final : public ca::sv::impl::methods<ca::sv::basic<C>, C> {


			private:

				// -- private types ------------------------------------------

				using self = ca::sv::basic<C>;


				// -- private constants --------------------------------------

				static constexpr C _empty[1] = {};


			public:

				// -- public types -------------------------------------------

				using value_type     = C;
				using size_type      = ca::usz;
				using const_iterator = const C*;


			private:

				// -- private members ----------------------------------------

				const C* _data;
				ca::usz _size;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr basic(void) noexcept
				: _data{self::_empty}, _size{0U} {
				}

				/* @DOC A NUL-terminated string, NUL excluded. */
				constexpr basic(const C* const text) noexcept
				: _data{text}, _size{ca::string_length(text)} {
				}

				constexpr basic(const C* const data, const ca::usz size) noexcept
				: _data{data}, _size{size} {
				}


				// -- iterators ----------------------------------------------

				[[nodiscard]] constexpr auto begin(void) const noexcept -> const C* {
					return _data;
				}

				[[nodiscard]] constexpr auto end(void) const noexcept -> const C* {
					return _data + _size;
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto data(void) const noexcept -> const C* {
					return _data;
				}

				/* @DOC Length in code units. */
				[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
					return _size;
				}

				[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
					return _size == 0U;
				}


				// -- element access -----------------------------------------

				/* @DOC Code unit at index. Traps when index >= size(), in every build. */
				[[nodiscard]] constexpr auto operator[](const ca::usz index) const noexcept -> const C& {

					if (index >= _size) [[unlikely]]
						ca::impl::index_out_of_bounds();
					return _data[index];
				}

				/* @DOC Code unit at index, or nothing when index >= size(). */
				[[nodiscard]] constexpr auto get(const ca::usz index) const noexcept -> ca::optional<const C&> {

					if (index >= _size)
						return ca::nullopt;
					return ca::optional<const C&>{_data[index]};
				}


				// -- methods ------------------------------------------------

				/* @DOC Itself: what the shared methods work on. */
				[[nodiscard]] constexpr auto view(void) const noexcept -> self {
					return *this;
				}

				[[nodiscard]] constexpr auto operator==(const self other) const noexcept -> bool {
					return _size == other._size && ca::equal_units(_data, other._data, _size);
				}

		}; // class basic

	} // namespace sv

} // namespace ca

#endif // ca_vocabulary_sv_basic_hpp
