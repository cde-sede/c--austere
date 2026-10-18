#ifndef ca_primitives_traits_is_trivially_assignable_hpp
#define ca_primitives_traits_is_trivially_assignable_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS TRIVIALLY ASSIGNABLE --------------------------------------------

	/* @DOC std::is_trivially_assignable_v, as a concept. */
	template <typename T, typename U>
	concept is_trivially_assignable = __is_trivially_assignable(T, U);

} // namespace ca

#endif // ca_primitives_traits_is_trivially_assignable_hpp
