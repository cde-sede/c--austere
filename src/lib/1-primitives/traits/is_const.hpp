#ifndef ca_primitives_traits_is_const_hpp
#define ca_primitives_traits_is_const_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS CONST -----------------------------------------------------------

	namespace impl {

		template <typename T>
		inline constexpr bool is_const = false;

		template <typename T>
		inline constexpr bool is_const<const T> = true;

	} // namespace impl


	/* @DOC std::is_const_v, as a concept. */
	template <typename T>
	concept is_const = ca::impl::is_const<T>;

} // namespace ca

#endif // ca_primitives_traits_is_const_hpp
