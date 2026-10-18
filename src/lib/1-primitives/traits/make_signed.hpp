#ifndef ca_primitives_traits_make_signed_hpp
#define ca_primitives_traits_make_signed_hpp

#include "1-primitives/traits/conditional.hpp"
#include "1-primitives/traits/impl/copy_cv.hpp"
#include "1-primitives/traits/impl/is_any_of.hpp"
#include "1-primitives/traits/is_enum.hpp"
#include "1-primitives/traits/is_integral.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MAKE SIGNED --------------------------------------------------------

	namespace impl {

		template <typename T>
		using make_signed_sized = ca::conditional<sizeof(T) == sizeof(signed char), signed char,
			ca::conditional<sizeof(T) == sizeof(short), short,
			ca::conditional<sizeof(T) == sizeof(int), int,
			ca::conditional<sizeof(T) == sizeof(long), long,
			long long>>>>;

		template <typename T>
		using make_signed = ca::conditional<ca::impl::is_any_of<T, char, signed char, unsigned char>, signed char,
			ca::conditional<ca::impl::is_any_of<T, short, unsigned short>, short,
			ca::conditional<ca::impl::is_any_of<T, int, unsigned int>, int,
			ca::conditional<ca::impl::is_any_of<T, long, unsigned long>, long,
			ca::conditional<ca::impl::is_any_of<T, long long, unsigned long long>, long long,
			ca::impl::make_signed_sized<T>>>>>>;

	} // namespace impl


	/* @DOC std::make_signed_t. */
	template <typename T>
	requires (ca::is_integral<T> && !ca::is_same<ca::remove_cv<T>, bool>) || ca::is_enum<T>
	using make_signed = ca::impl::copy_cv<T, ca::impl::make_signed<ca::remove_cv<T>>>;

} // namespace ca

#endif // ca_primitives_traits_make_signed_hpp
