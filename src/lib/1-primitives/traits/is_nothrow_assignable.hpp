#ifndef ca_primitives_traits_is_nothrow_assignable_hpp
#define ca_primitives_traits_is_nothrow_assignable_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS NOTHROW ASSIGNABLE ----------------------------------------------

	/* @DOC std::is_nothrow_assignable_v, as a concept. */
	template <typename T, typename U>
	concept is_nothrow_assignable = __is_nothrow_assignable(T, U);

} // namespace ca

#endif // ca_primitives_traits_is_nothrow_assignable_hpp
