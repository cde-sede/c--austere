#ifndef ca_lifecycle_is_clonable_hpp
#define ca_lifecycle_is_clonable_hpp

#include "1-primitives/traits/is_same.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS CLONABLE --------------------------------------------------------

	/* @DOC Types with explicit, fallible copy: clone() const -> result<T>. Containers prefer it
	   over the copy constructor. */
	template <typename T>
	concept is_clonable = requires(const T& value) {
		{ value.clone() } -> ca::is_same<ca::result<T>>;
	};

} // namespace ca

#endif // ca_lifecycle_is_clonable_hpp
