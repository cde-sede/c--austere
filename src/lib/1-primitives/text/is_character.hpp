#ifndef ca_primitives_text_is_character_hpp
#define ca_primitives_text_is_character_hpp

#include "1-primitives/traits/impl/is_any_of.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS CHARACTER -------------------------------------------------------

	/* @DOC The code unit types strings are made of: char and char8_t (UTF-8), char16_t (UTF-16),
	   char32_t (UTF-32), wchar_t (UTF-32 where it has 4 bytes, UTF-16 where it has 2). */
	template <typename T>
	concept is_character = ca::impl::is_any_of<T, char, char8_t, char16_t, char32_t, wchar_t>;

} // namespace ca

#endif // ca_primitives_text_is_character_hpp
