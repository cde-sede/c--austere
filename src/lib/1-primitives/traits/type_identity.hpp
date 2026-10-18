#ifndef ca_primitives_traits_type_identity_hpp
#define ca_primitives_traits_type_identity_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TYPE IDENTITY ------------------------------------------------------

	namespace impl {

		template <typename T>
		struct type_identity final {
				using self = ca::impl::type_identity<T>;
				using type = T;
				type_identity(void) = delete;
		}; // struct type_identity

	} // namespace impl


	/* @DOC std::type_identity_t. */
	template <typename T>
	using type_identity = typename ca::impl::type_identity<T>::type;

} // namespace ca

#endif // ca_primitives_traits_type_identity_hpp
