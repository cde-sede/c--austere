#ifndef ca_primitives_traits_unwrap_reference_hpp
#define ca_primitives_traits_unwrap_reference_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNWRAP REFERENCE ---------------------------------------------------

	namespace impl {

		template <typename T>
		struct unwrap_reference final {
				using self = ca::impl::unwrap_reference<T>;
				using type = T;
				unwrap_reference(void) = delete;
		}; // struct unwrap_reference

	} // namespace impl


	/* @DOC std::unwrap_reference_t. Identity until a reference wrapper specializes
	   ca::impl::unwrap_reference. */
	template <typename T>
	using unwrap_reference = typename ca::impl::unwrap_reference<T>::type;

} // namespace ca

#endif // ca_primitives_traits_unwrap_reference_hpp
