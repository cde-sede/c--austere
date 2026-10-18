#ifndef ca_primitives_traits_is_unbounded_array_hpp
#define ca_primitives_traits_is_unbounded_array_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS UNBOUNDED ARRAY -------------------------------------------------

	namespace impl {

		template <typename T>
		inline constexpr bool is_unbounded_array = false;

		template <typename T>
		inline constexpr bool is_unbounded_array<T[]> = true;

	} // namespace impl


	/* @DOC std::is_unbounded_array_v, as a concept. */
	template <typename T>
	concept is_unbounded_array = ca::impl::is_unbounded_array<T>;

} // namespace ca

#endif // ca_primitives_traits_is_unbounded_array_hpp
