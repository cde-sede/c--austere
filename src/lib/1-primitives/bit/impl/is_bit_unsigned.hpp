#ifndef ca_primitives_bit_impl_is_bit_unsigned_hpp
#define ca_primitives_bit_impl_is_bit_unsigned_hpp

#include "1-primitives/traits/impl/is_any_of.hpp"
#include "1-primitives/traits/is_unsigned.hpp"
#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS BIT UNSIGNED ----------------------------------------------------

	namespace impl {

		template <typename T>
		concept is_bit_unsigned = ca::is_unsigned<T>
			&& !ca::impl::is_any_of<ca::remove_cv<T>, bool, char, char8_t, char16_t, char32_t, wchar_t>;

	} // namespace impl

} // namespace ca

#endif // ca_primitives_bit_impl_is_bit_unsigned_hpp
