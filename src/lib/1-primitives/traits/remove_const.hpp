#ifndef ca_primitives_traits_remove_const_hpp
#define ca_primitives_traits_remove_const_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REMOVE CONST -------------------------------------------------------

	namespace impl {

		template <typename T>
		struct remove_const final {
				using self = ca::impl::remove_const<T>;
				using type = T;
				remove_const(void) = delete;
		}; // struct remove_const

		template <typename T>
		struct remove_const<const T> final {
				using self = ca::impl::remove_const<const T>;
				using type = T;
				remove_const(void) = delete;
		}; // struct remove_const

	} // namespace impl


	/* @DOC std::remove_const_t. */
	template <typename T>
	using remove_const = typename ca::impl::remove_const<T>::type;

} // namespace ca

#endif // ca_primitives_traits_remove_const_hpp
