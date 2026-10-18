#ifndef ca_primitives_traits_is_assignable_hpp
#define ca_primitives_traits_is_assignable_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS ASSIGNABLE ------------------------------------------------------

	/* @DOC std::is_assignable_v, as a concept. */
	template <typename T, typename U>
	concept is_assignable = __is_assignable(T, U);

} // namespace ca

#endif // ca_primitives_traits_is_assignable_hpp
