#ifndef ca_primitives_traits_is_integral_hpp
#define ca_primitives_traits_is_integral_hpp

#include "1-primitives/traits/impl/is_any_of.hpp"
#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS INTEGRAL --------------------------------------------------------

	/* @DOC std::is_integral_v, as a concept. */
	template <typename T>
	concept is_integral = ca::impl::is_any_of<ca::remove_cv<T>,
		bool, char, signed char, unsigned char, wchar_t, char8_t, char16_t, char32_t,
		short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long>;

} // namespace ca

#endif // ca_primitives_traits_is_integral_hpp
