#ifndef ca_primitives_traits_has_virtual_destructor_hpp
#define ca_primitives_traits_has_virtual_destructor_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- HAS VIRTUAL DESTRUCTOR ---------------------------------------------

	/* @DOC std::has_virtual_destructor_v, as a concept. */
	template <typename T> // concept-name: ignore
	concept has_virtual_destructor = __has_virtual_destructor(T);

} // namespace ca

#endif // ca_primitives_traits_has_virtual_destructor_hpp
