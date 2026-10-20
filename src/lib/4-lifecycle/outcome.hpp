#ifndef ca_lifecycle_outcome_hpp
#define ca_lifecycle_outcome_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- OUTCOME ------------------------------------------------------------

	/* @DOC How a context ended, for a leave that asks (leave(error, outcome)): enter succeeded
	   (error is nullptr), the context took the failure and run returns it, or a trap passed it
	   for an outer context and run never returns. */
	enum class outcome {
		succeeded,
		taken,
		passed,
	};

} // namespace ca

#endif // ca_lifecycle_outcome_hpp
