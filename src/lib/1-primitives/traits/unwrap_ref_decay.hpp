#ifndef ca_primitives_traits_unwrap_ref_decay_hpp
#define ca_primitives_traits_unwrap_ref_decay_hpp

#include "1-primitives/traits/decay.hpp"
#include "1-primitives/traits/unwrap_reference.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNWRAP REF DECAY ---------------------------------------------------

	/* @DOC std::unwrap_ref_decay_t. */
	template <typename T>
	using unwrap_ref_decay = ca::unwrap_reference<ca::decay<T>>;

} // namespace ca

#endif // ca_primitives_traits_unwrap_ref_decay_hpp
