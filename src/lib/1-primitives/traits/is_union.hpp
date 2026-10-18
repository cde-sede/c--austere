#ifndef ca_primitives_traits_is_union_hpp
#define ca_primitives_traits_is_union_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS UNION -----------------------------------------------------------

	/* @DOC std::is_union_v, as a concept. */
	template <typename T>
	concept is_union = __is_union(T);

} // namespace ca

#endif // ca_primitives_traits_is_union_hpp
