#ifndef ca_primitives_traits_make_unsigned_hpp
#define ca_primitives_traits_make_unsigned_hpp

#include "1-primitives/traits/conditional.hpp"
#include "1-primitives/traits/impl/copy_cv.hpp"
#include "1-primitives/traits/impl/is_any_of.hpp"
#include "1-primitives/traits/is_enum.hpp"
#include "1-primitives/traits/is_integral.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MAKE UNSIGNED ------------------------------------------------------

	namespace impl {

		template <typename T>
		using make_unsigned_sized = ca::conditional<sizeof(T) == sizeof(unsigned char), unsigned char,
			ca::conditional<sizeof(T) == sizeof(unsigned short), unsigned short,
			ca::conditional<sizeof(T) == sizeof(unsigned int), unsigned int,
			ca::conditional<sizeof(T) == sizeof(unsigned long), unsigned long,
			unsigned long long>>>>;

		template <typename T>
		using make_unsigned = ca::conditional<ca::impl::is_any_of<T, char, signed char, unsigned char>, unsigned char,
			ca::conditional<ca::impl::is_any_of<T, short, unsigned short>, unsigned short,
			ca::conditional<ca::impl::is_any_of<T, int, unsigned int>, unsigned int,
			ca::conditional<ca::impl::is_any_of<T, long, unsigned long>, unsigned long,
			ca::conditional<ca::impl::is_any_of<T, long long, unsigned long long>, unsigned long long,
			ca::impl::make_unsigned_sized<T>>>>>>;

	} // namespace impl


	/* @DOC std::make_unsigned_t. */
	template <typename T>
	requires (ca::is_integral<T> && !ca::is_same<ca::remove_cv<T>, bool>) || ca::is_enum<T>
	using make_unsigned = ca::impl::copy_cv<T, ca::impl::make_unsigned<ca::remove_cv<T>>>;

} // namespace ca

#endif // ca_primitives_traits_make_unsigned_hpp
