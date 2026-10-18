#ifndef ca_primitives_traits_is_base_of_hpp
#define ca_primitives_traits_is_base_of_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS BASE OF ---------------------------------------------------------

	/* @DOC std::is_base_of_v, as a concept. */
	template <typename B, typename D>
	concept is_base_of = __is_base_of(B, D);

} // namespace ca

#endif // ca_primitives_traits_is_base_of_hpp
