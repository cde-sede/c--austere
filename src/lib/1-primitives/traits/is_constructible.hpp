#ifndef ca_primitives_traits_is_constructible_hpp
#define ca_primitives_traits_is_constructible_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS CONSTRUCTIBLE ---------------------------------------------------

	/* @DOC std::is_constructible_v, as a concept. */
	template <typename T, typename... Args>
	concept is_constructible = __is_constructible(T, Args...);

} // namespace ca

#endif // ca_primitives_traits_is_constructible_hpp
