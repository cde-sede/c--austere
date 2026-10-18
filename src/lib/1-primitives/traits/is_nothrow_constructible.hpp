#ifndef ca_primitives_traits_is_nothrow_constructible_hpp
#define ca_primitives_traits_is_nothrow_constructible_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS NOTHROW CONSTRUCTIBLE -------------------------------------------

	/* @DOC std::is_nothrow_constructible_v, as a concept. */
	template <typename T, typename... Args>
	concept is_nothrow_constructible = __is_nothrow_constructible(T, Args...);

} // namespace ca

#endif // ca_primitives_traits_is_nothrow_constructible_hpp
