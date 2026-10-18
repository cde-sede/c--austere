#ifndef ca_primitives_traits_is_standard_layout_hpp
#define ca_primitives_traits_is_standard_layout_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS STANDARD LAYOUT -------------------------------------------------

	/* @DOC std::is_standard_layout_v, as a concept. */
	template <typename T>
	concept is_standard_layout = __is_standard_layout(T);

} // namespace ca

#endif // ca_primitives_traits_is_standard_layout_hpp
