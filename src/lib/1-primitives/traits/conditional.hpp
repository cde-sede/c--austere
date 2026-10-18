#ifndef ca_primitives_traits_conditional_hpp
#define ca_primitives_traits_conditional_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- CONDITIONAL --------------------------------------------------------

	namespace impl {

		template <bool B, typename T, typename F>
		struct conditional final {
				using self = ca::impl::conditional<B, T, F>;
				using type = T;
				conditional(void) = delete;
		}; // struct conditional

		template <typename T, typename F>
		struct conditional<false, T, F> final {
				using self = ca::impl::conditional<false, T, F>;
				using type = F;
				conditional(void) = delete;
		}; // struct conditional

	} // namespace impl


	/* @DOC std::conditional_t. */
	template <bool B, typename T, typename F>
	using conditional = typename ca::impl::conditional<B, T, F>::type;

} // namespace ca

#endif // ca_primitives_traits_conditional_hpp
