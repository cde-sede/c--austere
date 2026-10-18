#ifndef ca_primitives_traits_is_trivial_hpp
#define ca_primitives_traits_is_trivial_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS TRIVIAL ---------------------------------------------------------

	/* @DOC std::is_trivial_v, as a concept. Deprecated in std, kept for parity. */
	template <typename T>
	concept is_trivial = __is_trivial(T);

} // namespace ca

#endif // ca_primitives_traits_is_trivial_hpp
