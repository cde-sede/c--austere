#ifndef ca_primitives_traits_is_abstract_hpp
#define ca_primitives_traits_is_abstract_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS ABSTRACT --------------------------------------------------------

	/* @DOC std::is_abstract_v, as a concept. */
	template <typename T>
	concept is_abstract = __is_abstract(T);

} // namespace ca

#endif // ca_primitives_traits_is_abstract_hpp
