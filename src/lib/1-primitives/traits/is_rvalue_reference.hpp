#ifndef ca_primitives_traits_is_rvalue_reference_hpp
#define ca_primitives_traits_is_rvalue_reference_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS RVALUE REFERENCE ------------------------------------------------

	namespace impl {

		template <typename T>
		inline constexpr bool is_rvalue_reference = false;

		template <typename T>
		inline constexpr bool is_rvalue_reference<T&&> = true;

	} // namespace impl


	/* @DOC std::is_rvalue_reference_v, as a concept. */
	template <typename T>
	concept is_rvalue_reference = ca::impl::is_rvalue_reference<T>;

} // namespace ca

#endif // ca_primitives_traits_is_rvalue_reference_hpp
