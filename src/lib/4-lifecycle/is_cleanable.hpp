#ifndef ca_lifecycle_is_cleanable_hpp
#define ca_lifecycle_is_cleanable_hpp

#include "1-primitives/traits/is_same.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS CLEANABLE -------------------------------------------------------

	/* @DOC Types with explicit, fallible cleanup: destroy() -> result<void>. Their destructor is only
	   a failsafe for a skipped destroy(). */
	template <typename T>
	concept is_cleanable = requires(T& value) {
		{ value.destroy() } -> ca::is_same<ca::result<void>>;
	};

} // namespace ca

#endif // ca_lifecycle_is_cleanable_hpp
