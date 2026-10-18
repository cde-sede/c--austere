#ifndef ca_primitives_traits_basic_common_reference_hpp
#define ca_primitives_traits_basic_common_reference_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BASIC COMMON REFERENCE ---------------------------------------------

	/* @DOC std::basic_common_reference. Customization point: specialize it with a member type. */
	template <typename T, typename U, template <typename> typename TQ, template <typename> typename UQ>
	struct basic_common_reference final {
			using self = ca::basic_common_reference<T, U, TQ, UQ>;
			basic_common_reference(void) = delete;
	}; // struct basic_common_reference

} // namespace ca

#endif // ca_primitives_traits_basic_common_reference_hpp
