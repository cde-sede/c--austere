#ifndef ca_primitives_traits_add_rvalue_reference_hpp
#define ca_primitives_traits_add_rvalue_reference_hpp

#include "1-primitives/traits/impl/is_referenceable.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ADD RVALUE REFERENCE -----------------------------------------------

	namespace impl {

		template <typename T>
		struct add_rvalue_reference final {
				using self = ca::impl::add_rvalue_reference<T>;
				using type = T;
				add_rvalue_reference(void) = delete;
		}; // struct add_rvalue_reference

		template <typename T>
		requires ca::impl::is_referenceable<T>
		struct add_rvalue_reference<T> final {
				using self = ca::impl::add_rvalue_reference<T>;
				using type = T&&;
				add_rvalue_reference(void) = delete;
		}; // struct add_rvalue_reference

	} // namespace impl


	/* @DOC std::add_rvalue_reference_t. */
	template <typename T>
	using add_rvalue_reference = typename ca::impl::add_rvalue_reference<T>::type;

} // namespace ca

#endif // ca_primitives_traits_add_rvalue_reference_hpp
