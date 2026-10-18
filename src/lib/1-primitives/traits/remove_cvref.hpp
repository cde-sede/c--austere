#ifndef ca_primitives_traits_remove_cvref_hpp
#define ca_primitives_traits_remove_cvref_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REMOVE CVREF -------------------------------------------------------

	/* @DOC std::remove_cvref_t. */
	template <typename T>
	using remove_cvref = __remove_cvref(T);

} // namespace ca

#endif // ca_primitives_traits_remove_cvref_hpp
