#ifndef ca_primitives_traits_is_layout_compatible_hpp
#define ca_primitives_traits_is_layout_compatible_hpp

#include "1-primitives/config/has_builtin.hpp"
#include "1-primitives/traits/is_class.hpp"
#include "1-primitives/traits/is_enum.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/is_union.hpp"
#include "1-primitives/traits/remove_cv.hpp"
#include "1-primitives/traits/underlying_type.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS LAYOUT COMPATIBLE -----------------------------------------------

	/* @DOC std::is_layout_compatible_v, as a concept. Without __is_layout_compatible (clang 18):
	   distinct class types fail to compile with a ca: message. */
#if ca_has_builtin(__is_layout_compatible)
	template <typename T, typename U>
	concept is_layout_compatible = __is_layout_compatible(T, U);
#else
	namespace impl {

		template <typename T, typename U>
		concept are_distinct_classes = (ca::is_class<T> || ca::is_union<T>) && (ca::is_class<U> || ca::is_union<U>)
			&& !ca::is_same<ca::remove_cv<T>, ca::remove_cv<U>>;

		template <typename T, typename U>
		struct layout_unknown final {
				using self = ca::impl::layout_unknown<T, U>;
				static_assert(false, "ca: is_layout_compatible on distinct class types needs __is_layout_compatible");
				static constexpr bool value = false;
				layout_unknown(void) = delete;
		}; // struct layout_unknown

	} // namespace impl


	template <typename T, typename U>
	concept is_layout_compatible = ca::is_same<ca::remove_cv<T>, ca::remove_cv<U>>
		|| (ca::is_enum<T> && ca::is_enum<U> && ca::is_same<ca::underlying_type<T>, ca::underlying_type<U>>)
		|| (ca::impl::are_distinct_classes<T, U> && ca::impl::layout_unknown<T, U>::value);
#endif

} // namespace ca

#endif // ca_primitives_traits_is_layout_compatible_hpp
