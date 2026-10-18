#ifndef ca_collections_format_impl_display_argument_hpp
#define ca_collections_format_impl_display_argument_hpp

#include "1-primitives/floats/f64.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/find_unit.hpp"
#include "1-primitives/memory/string_length.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_enum.hpp"
#include "1-primitives/traits/is_null_pointer.hpp"
#include "1-primitives/traits/is_pointer.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_const.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/traits/remove_extent.hpp"
#include "1-primitives/traits/remove_pointer.hpp"
#include "1-primitives/traits/underlying_type.hpp"
#include "1-primitives/utility/addressof.hpp"
#include "5-collections/format/impl/display_kind.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DISPLAY ARGUMENT ---------------------------------------------------

	namespace impl {

		struct display_text final {

				using self = ca::impl::display_text;

				const char* _data;
				ca::usz _size;

		}; // struct display_text

		/* @DOC Text of 2- or 4-byte code units; the argument's size is the unit size, with
		   wide_text_flag for wchar_t. */
		struct display_units final {

				using self = ca::impl::display_units;

				const void* _data;
				ca::usz _size;

		}; // struct display_units

		struct display_custom final {

				using self = ca::impl::display_custom;

				using display_function = auto (*)(const void*, ca::text_writer&) noexcept -> void;

				const void* _object;
				display_function _display;

		}; // struct display_custom


		/* @DOC One argument of print, its type erased: the engine is not a template. size: bytes
		   of the source integer, float or character type, or of a wide text's code unit. */
		struct display_argument final {

				using self = ca::impl::display_argument;

				union {
						ca::u64 _unsigned_value;
						ca::i64 _signed_value;
						ca::f64 _float_value;
						const void* _pointer;
						ca::impl::display_text _text;
						ca::impl::display_units _units;
						ca::impl::display_custom _custom;
						int _code;
						bool _boolean;
						char _character;
				};

				ca::impl::display_kind _kind;
				ca::u8 _size;

		}; // struct display_argument


		template <typename T>
		auto display_thunk(const void* const object, ca::text_writer& out) noexcept -> void {
			static_cast<const T*>(object)->display(out);
		}

		inline constexpr ca::u8 wide_text_flag = 0x80U;

		template <typename C>
		inline auto make_display_text(ca::impl::display_argument& argument, const C* const data, const ca::usz size) noexcept -> void {
			if constexpr (sizeof(C) == 1U)
				argument._text = ca::impl::display_text{reinterpret_cast<const char*>(data), size};
			else {
				argument._units = ca::impl::display_units{data, size};
				 argument._size = static_cast<ca::u8>(sizeof(C) | (ca::is_same<C, wchar_t> ? ca::impl::wide_text_flag : 0U));
			}
		}

		template <typename T>
		[[nodiscard]] inline auto make_display_argument(const T& value) noexcept -> ca::impl::display_argument {

			using U = ca::remove_cvref<T>;
			constexpr ca::impl::display_kind kind = ca::impl::display_kind_of<U>;
			static_assert(kind != ca::impl::display_kind::none, "ca::print: argument type is not displayable");
			ca::impl::display_argument argument{};
			argument._kind = kind;
			argument._size = static_cast<ca::u8>(sizeof(U));
			if constexpr (kind == ca::impl::display_kind::custom)
				argument._custom = ca::impl::display_custom{ca::addressof(value), &ca::impl::display_thunk<U>};
			else if constexpr (ca::is_enum<U>)
				return ca::impl::make_display_argument(static_cast<ca::underlying_type<U>>(value));
			else if constexpr (kind == ca::impl::display_kind::boolean)
				argument._boolean = value;
			else if constexpr (kind == ca::impl::display_kind::character)
				argument._character = static_cast<char>(value);
			else if constexpr (kind == ca::impl::display_kind::signed_integer)
				argument._signed_value = static_cast<ca::i64>(value);
			else if constexpr (kind == ca::impl::display_kind::unsigned_integer)
				argument._unsigned_value = static_cast<ca::u64>(value);
			else if constexpr (kind == ca::impl::display_kind::floating)
				argument._float_value = static_cast<ca::f64>(value);
			else if constexpr (kind == ca::impl::display_kind::error)
				argument._code = value.code();
			else if constexpr (kind == ca::impl::display_kind::pointer) {
				if constexpr (ca::is_null_pointer<U>)
					argument._pointer = nullptr;
				else
					argument._pointer = const_cast<const void*>(static_cast<const volatile void*>(value));
			}
			else if constexpr (kind == ca::impl::display_kind::code_point)
				argument._unsigned_value = static_cast<ca::u32>(value);
			else if constexpr (ca::is_array<U>) {
				using unit = ca::remove_const<ca::remove_extent<U>>;
				constexpr ca::usz extent = sizeof(U) / sizeof(unit);
				const unit* const end = ca::find_unit(value, unit{}, extent);
				ca::impl::make_display_text(argument, value, end != nullptr ? static_cast<ca::usz>(end - value) : extent);
			}
			else if constexpr (ca::is_pointer<U>) {
				ca::impl::make_display_text(argument, value, value != nullptr ? ca::string_length(value) : 0U);
			}
			else if constexpr (ca::impl::is_text_view<U>)
				ca::impl::make_display_text(argument, value.data(), value.size());
			else {
				const auto view = value.view();
				ca::impl::make_display_text(argument, view.data(), view.size());
			}
			return argument;
		}

	} // namespace impl

} // namespace ca

#endif // ca_collections_format_impl_display_argument_hpp
