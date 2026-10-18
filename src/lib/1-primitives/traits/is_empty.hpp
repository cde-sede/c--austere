#ifndef ca_primitives_traits_is_empty_hpp
#define ca_primitives_traits_is_empty_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS EMPTY -----------------------------------------------------------

	/* @DOC std::is_empty_v, as a concept. */
	template <typename T>
	concept is_empty = __is_empty(T);

} // namespace ca

#endif // ca_primitives_traits_is_empty_hpp
