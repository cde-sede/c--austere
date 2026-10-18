#ifndef ca_primitives_traits_remove_cv_hpp
#define ca_primitives_traits_remove_cv_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REMOVE CV ----------------------------------------------------------

	/* @DOC std::remove_cv_t. */
	template <typename T>
	using remove_cv = __remove_cv(T);

} // namespace ca

#endif // ca_primitives_traits_remove_cv_hpp
