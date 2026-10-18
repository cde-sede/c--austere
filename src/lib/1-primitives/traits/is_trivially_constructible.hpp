#ifndef ca_primitives_traits_is_trivially_constructible_hpp
#define ca_primitives_traits_is_trivially_constructible_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS TRIVIALLY CONSTRUCTIBLE -----------------------------------------

	/* @DOC std::is_trivially_constructible_v, as a concept. */
	template <typename T, typename... Args>
	concept is_trivially_constructible = __is_trivially_constructible(T, Args...);

} // namespace ca

#endif // ca_primitives_traits_is_trivially_constructible_hpp
