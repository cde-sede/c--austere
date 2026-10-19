#ifndef ca_lifecycle_is_cleanable_hpp
#define ca_lifecycle_is_cleanable_hpp

#include "1-primitives/traits/is_same.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS CLEANABLE -------------------------------------------------------

	/* @DOC Types owning resources. Contract: cleanup() -> result<void> releases every resource the
	   object owns; other methods (close, flush, ...) may do part of it. Only owners have cleanup().
	   Their destructor is only a failsafe for a skipped cleanup(). */
	template <typename T>
	concept is_cleanable = requires(T& value) {
		{ value.cleanup() } -> ca::is_same<ca::result<void>>;
	};

} // namespace ca

#endif // ca_lifecycle_is_cleanable_hpp
