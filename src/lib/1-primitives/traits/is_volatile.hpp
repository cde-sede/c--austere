#ifndef ca_primitives_traits_is_volatile_hpp
#define ca_primitives_traits_is_volatile_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS VOLATILE --------------------------------------------------------

	namespace impl {

		template <typename T>
		inline constexpr bool is_volatile = false;

		template <typename T>
		inline constexpr bool is_volatile<volatile T> = true;

	} // namespace impl


	/* @DOC std::is_volatile_v, as a concept. */
	template <typename T>
	concept is_volatile = ca::impl::is_volatile<T>;

} // namespace ca

#endif // ca_primitives_traits_is_volatile_hpp
