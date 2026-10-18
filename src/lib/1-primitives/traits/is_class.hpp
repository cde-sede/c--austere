#ifndef ca_primitives_traits_is_class_hpp
#define ca_primitives_traits_is_class_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS CLASS -----------------------------------------------------------

	/* @DOC std::is_class_v, as a concept. */
	template <typename T>
	concept is_class = __is_class(T);

} // namespace ca

#endif // ca_primitives_traits_is_class_hpp
