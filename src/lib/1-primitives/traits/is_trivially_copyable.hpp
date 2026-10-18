#ifndef ca_primitives_traits_is_trivially_copyable_hpp
#define ca_primitives_traits_is_trivially_copyable_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS TRIVIALLY COPYABLE ----------------------------------------------

	/* @DOC std::is_trivially_copyable_v, as a concept. */
	template <typename T>
	concept is_trivially_copyable = __is_trivially_copyable(T);

} // namespace ca

#endif // ca_primitives_traits_is_trivially_copyable_hpp
