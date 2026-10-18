#ifndef ca_primitives_traits_remove_reference_hpp
#define ca_primitives_traits_remove_reference_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REMOVE REFERENCE ---------------------------------------------------

	namespace impl {

		template <typename T>
		struct remove_reference final {
				using self = ca::impl::remove_reference<T>;
				using type = T;
				remove_reference(void) = delete;
		}; // struct remove_reference

		template <typename T>
		struct remove_reference<T&> final {
				using self = ca::impl::remove_reference<T&>;
				using type = T;
				remove_reference(void) = delete;
		}; // struct remove_reference

		template <typename T>
		struct remove_reference<T&&> final {
				using self = ca::impl::remove_reference<T&&>;
				using type = T;
				remove_reference(void) = delete;
		}; // struct remove_reference

	} // namespace impl


	/* @DOC std::remove_reference_t. */
	template <typename T>
	using remove_reference = typename ca::impl::remove_reference<T>::type;

} // namespace ca

#endif // ca_primitives_traits_remove_reference_hpp
