#ifndef ca_primitives_traits_remove_pointer_hpp
#define ca_primitives_traits_remove_pointer_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REMOVE POINTER -----------------------------------------------------

	namespace impl {

		template <typename T>
		struct remove_pointer final {
				using self = ca::impl::remove_pointer<T>;
				using type = T;
				remove_pointer(void) = delete;
		}; // struct remove_pointer

		template <typename T>
		struct remove_pointer<T*> final {
				using self = ca::impl::remove_pointer<T*>;
				using type = T;
				remove_pointer(void) = delete;
		}; // struct remove_pointer

		template <typename T>
		struct remove_pointer<T* const> final {
				using self = ca::impl::remove_pointer<T* const>;
				using type = T;
				remove_pointer(void) = delete;
		}; // struct remove_pointer

		template <typename T>
		struct remove_pointer<T* volatile> final {
				using self = ca::impl::remove_pointer<T* volatile>;
				using type = T;
				remove_pointer(void) = delete;
		}; // struct remove_pointer

		template <typename T>
		struct remove_pointer<T* const volatile> final {
				using self = ca::impl::remove_pointer<T* const volatile>;
				using type = T;
				remove_pointer(void) = delete;
		}; // struct remove_pointer

	} // namespace impl


	/* @DOC std::remove_pointer_t. */
	template <typename T>
	using remove_pointer = typename ca::impl::remove_pointer<T>::type;

} // namespace ca

#endif // ca_primitives_traits_remove_pointer_hpp
