#ifndef ca_primitives_traits_is_convertible_hpp
#define ca_primitives_traits_is_convertible_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS CONVERTIBLE -----------------------------------------------------

	/* @DOC std::is_convertible_v, as a concept. */
	template <typename From, typename To>
	concept is_convertible = __is_convertible(From, To);

} // namespace ca

#endif // ca_primitives_traits_is_convertible_hpp
