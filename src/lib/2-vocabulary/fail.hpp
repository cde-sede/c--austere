#ifndef ca_vocabulary_fail_hpp
#define ca_vocabulary_fail_hpp

#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FAIL ---------------------------------------------------------------

	/* @DOC ca::failure{ca::error{code}}. */
	[[nodiscard]] constexpr auto fail(const int code) noexcept -> ca::failure<ca::error> {
		return ca::failure<ca::error>{ca::error{code}};
	}

} // namespace ca

#endif // ca_vocabulary_fail_hpp
