#ifndef ca_primitives_traits_is_pod_hpp
#define ca_primitives_traits_is_pod_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS POD -------------------------------------------------------------

	/* @DOC std::is_pod_v, as a concept. Deprecated in std, kept for parity. */
	template <typename T>
	concept is_pod = __is_pod(T);

} // namespace ca

#endif // ca_primitives_traits_is_pod_hpp
