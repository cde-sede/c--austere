#ifndef ca_primitives_traits_add_pointer_hpp
#define ca_primitives_traits_add_pointer_hpp

#include "1-primitives/traits/remove_reference.hpp"
#include "1-primitives/traits/type_identity.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ADD POINTER --------------------------------------------------------

	namespace impl {

		template <typename T>
		concept is_pointable = requires {
			typename ca::type_identity<ca::remove_reference<T>*>;
		};

		template <typename T>
		struct add_pointer final {
				using self = ca::impl::add_pointer<T>;
				using type = T;
				add_pointer(void) = delete;
		}; // struct add_pointer

		template <typename T>
		requires ca::impl::is_pointable<T>
		struct add_pointer<T> final {
				using self = ca::impl::add_pointer<T>;
				using type = ca::remove_reference<T>*;
				add_pointer(void) = delete;
		}; // struct add_pointer

	} // namespace impl


	/* @DOC std::add_pointer_t. */
	template <typename T>
	using add_pointer = typename ca::impl::add_pointer<T>::type;

} // namespace ca

#endif // ca_primitives_traits_add_pointer_hpp
