#ifndef ca_vocabulary_sv_impl_forward_hpp
#define ca_vocabulary_sv_impl_forward_hpp

#include "1-primitives/text/is_character.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SV -----------------------------------------------------------------

	namespace sv {


		// -- FORWARD --------------------------------------------------------

		/* @DOC sv::basic declared ahead: its methods use headers (ca::unicode) that take views. */
		template <ca::is_character C>
		class basic;

	} // namespace sv

} // namespace ca

#endif // ca_vocabulary_sv_impl_forward_hpp
