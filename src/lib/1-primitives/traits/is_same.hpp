#ifndef ca_primitives_traits_is_same_hpp
#define ca_primitives_traits_is_same_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS SAME ------------------------------------------------------------

	/* @DOC std::is_same_v, as a concept. */
	template <typename T, typename U>
	concept is_same = __is_same(T, U);

} // namespace ca

#endif // ca_primitives_traits_is_same_hpp
