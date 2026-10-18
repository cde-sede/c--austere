#ifndef ca_primitives_traits_enable_if_hpp
#define ca_primitives_traits_enable_if_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ENABLE IF ----------------------------------------------------------

	namespace impl {

		template <bool B, typename T>
		struct enable_if final {
				using self = ca::impl::enable_if<B, T>;
				enable_if(void) = delete;
		}; // struct enable_if

		template <typename T>
		struct enable_if<true, T> final {
				using self = ca::impl::enable_if<true, T>;
				using type = T;
				enable_if(void) = delete;
		}; // struct enable_if

	} // namespace impl


	/* @DOC std::enable_if_t. */
	template <bool B, typename T = void>
	using enable_if = typename ca::impl::enable_if<B, T>::type;

} // namespace ca

#endif // ca_primitives_traits_enable_if_hpp
