#ifndef ca_primitives_traits_is_lvalue_reference_hpp
#define ca_primitives_traits_is_lvalue_reference_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS LVALUE REFERENCE ------------------------------------------------

	namespace impl {

		template <typename T>
		inline constexpr bool is_lvalue_reference = false;

		template <typename T>
		inline constexpr bool is_lvalue_reference<T&> = true;

	} // namespace impl


	/* @DOC std::is_lvalue_reference_v, as a concept. */
	template <typename T>
	concept is_lvalue_reference = ca::impl::is_lvalue_reference<T>;

} // namespace ca

#endif // ca_primitives_traits_is_lvalue_reference_hpp
