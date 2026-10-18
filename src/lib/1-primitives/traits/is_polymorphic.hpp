#ifndef ca_primitives_traits_is_polymorphic_hpp
#define ca_primitives_traits_is_polymorphic_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS POLYMORPHIC -----------------------------------------------------

	/* @DOC std::is_polymorphic_v, as a concept. */
	template <typename T>
	concept is_polymorphic = __is_polymorphic(T);

} // namespace ca

#endif // ca_primitives_traits_is_polymorphic_hpp
