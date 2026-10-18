#ifndef ca_primitives_traits_add_cv_hpp
#define ca_primitives_traits_add_cv_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ADD CV -------------------------------------------------------------

	/* @DOC std::add_cv_t. */
	template <typename T>
	using add_cv = const volatile T;

} // namespace ca

#endif // ca_primitives_traits_add_cv_hpp
