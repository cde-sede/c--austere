#ifndef ca_primitives_integers_isz_hpp
#define ca_primitives_integers_isz_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ISZ ----------------------------------------------------------------

	/* @DOC Signed type of a pointer difference. */
	using isz = decltype(static_cast<char*>(nullptr) - static_cast<char*>(nullptr));

} // namespace ca

#endif // ca_primitives_integers_isz_hpp
