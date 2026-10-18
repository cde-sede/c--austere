#ifndef ca_primitives_traits_remove_volatile_hpp
#define ca_primitives_traits_remove_volatile_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REMOVE VOLATILE ----------------------------------------------------

	namespace impl {

		template <typename T>
		struct remove_volatile final {
				using self = ca::impl::remove_volatile<T>;
				using type = T;
				remove_volatile(void) = delete;
		}; // struct remove_volatile

		template <typename T>
		struct remove_volatile<volatile T> final {
				using self = ca::impl::remove_volatile<volatile T>;
				using type = T;
				remove_volatile(void) = delete;
		}; // struct remove_volatile

	} // namespace impl


	/* @DOC std::remove_volatile_t. */
	template <typename T>
	using remove_volatile = typename ca::impl::remove_volatile<T>::type;

} // namespace ca

#endif // ca_primitives_traits_remove_volatile_hpp
