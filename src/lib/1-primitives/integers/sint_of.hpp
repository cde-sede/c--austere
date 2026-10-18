#ifndef ca_primitives_integers_sint_of_hpp
#define ca_primitives_integers_sint_of_hpp

#include "1-primitives/integers/bits.hpp"
#include "1-primitives/traits/conditional.hpp"
#include "1-primitives/traits/is_void.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SINT OF ------------------------------------------------------------

	namespace impl {

		template <unsigned B>
		using sint_of = ca::conditional<ca::bits<signed char> == B, signed char,
			ca::conditional<ca::bits<signed short> == B, signed short,
			ca::conditional<ca::bits<signed int> == B, signed int,
			ca::conditional<ca::bits<signed long> == B, signed long,
			ca::conditional<ca::bits<signed long long> == B, signed long long, void>>>>>;

	} // namespace impl


	/* @DOC Signed standard integer type of exactly B bits; fails to compile if there is none. */
	template <unsigned B>
	requires (!ca::is_void<ca::impl::sint_of<B>>)
	using sint_of = ca::impl::sint_of<B>;

} // namespace ca

#endif // ca_primitives_integers_sint_of_hpp
