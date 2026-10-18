#ifndef ca_primitives_traits_conjunction_hpp
#define ca_primitives_traits_conjunction_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- CONJUNCTION --------------------------------------------------------

	namespace impl {

		template <typename... Bs>
		inline constexpr bool conjunction = true;

		template <bool V, typename... Bs>
		inline constexpr bool conjunction_from = false;

		template <typename... Bs>
		inline constexpr bool conjunction_from<true, Bs...> = ca::impl::conjunction<Bs...>;

		template <typename B, typename... Bs>
		inline constexpr bool conjunction<B, Bs...> = ca::impl::conjunction_from<static_cast<bool>(B::value), Bs...>;

	} // namespace impl


	/* @DOC std::conjunction_v, as a concept. Over types with ::value, short-circuiting. */
	template <typename... Bs> // concept-name: ignore
	concept conjunction = ca::impl::conjunction<Bs...>;

} // namespace ca

#endif // ca_primitives_traits_conjunction_hpp
