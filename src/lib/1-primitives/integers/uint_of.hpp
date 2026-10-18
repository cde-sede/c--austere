#ifndef ca_primitives_integers_uint_of_hpp
#define ca_primitives_integers_uint_of_hpp

#include "1-primitives/integers/bits.hpp"
#include "1-primitives/traits/conditional.hpp"
#include "1-primitives/traits/is_void.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UINT OF ------------------------------------------------------------

	namespace impl {

		template <unsigned B>
		using uint_of = ca::conditional<ca::bits<unsigned char> == B, unsigned char,
			ca::conditional<ca::bits<unsigned short> == B, unsigned short,
			ca::conditional<ca::bits<unsigned int> == B, unsigned int,
			ca::conditional<ca::bits<unsigned long> == B, unsigned long,
			ca::conditional<ca::bits<unsigned long long> == B, unsigned long long, void>>>>>;

	} // namespace impl


	/* @DOC Unsigned standard integer type of exactly B bits; fails to compile if there is none. */
	template <unsigned B>
	requires (!ca::is_void<ca::impl::uint_of<B>>)
	using uint_of = ca::impl::uint_of<B>;

} // namespace ca

#endif // ca_primitives_integers_uint_of_hpp
