#ifndef ca_primitives_traits_is_final_hpp
#define ca_primitives_traits_is_final_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS FINAL -----------------------------------------------------------

	/* @DOC std::is_final_v, as a concept. */
	template <typename T>
	concept is_final = __is_final(T);

} // namespace ca

#endif // ca_primitives_traits_is_final_hpp
