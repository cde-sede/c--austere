#ifndef ca_primitives_traits_is_enum_hpp
#define ca_primitives_traits_is_enum_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS ENUM ------------------------------------------------------------

	/* @DOC std::is_enum_v, as a concept. */
	template <typename T>
	concept is_enum = __is_enum(T);

} // namespace ca

#endif // ca_primitives_traits_is_enum_hpp
