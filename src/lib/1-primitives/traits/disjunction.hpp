#ifndef ca_primitives_traits_disjunction_hpp
#define ca_primitives_traits_disjunction_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DISJUNCTION --------------------------------------------------------

	namespace impl {

		template <typename... Bs>
		inline constexpr bool disjunction = false;

		template <bool V, typename... Bs>
		inline constexpr bool disjunction_from = true;

		template <typename... Bs>
		inline constexpr bool disjunction_from<false, Bs...> = ca::impl::disjunction<Bs...>;

		template <typename B, typename... Bs>
		inline constexpr bool disjunction<B, Bs...> = ca::impl::disjunction_from<static_cast<bool>(B::value), Bs...>;

	} // namespace impl


	/* @DOC std::disjunction_v, as a concept. Over types with ::value, short-circuiting. */
	template <typename... Bs> // concept-name: ignore
	concept disjunction = ca::impl::disjunction<Bs...>;

} // namespace ca

#endif // ca_primitives_traits_disjunction_hpp
