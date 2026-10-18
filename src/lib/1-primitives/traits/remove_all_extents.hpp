#ifndef ca_primitives_traits_remove_all_extents_hpp
#define ca_primitives_traits_remove_all_extents_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REMOVE ALL EXTENTS -------------------------------------------------

	namespace impl {

		template <typename T>
		struct remove_all_extents final {
				using self = ca::impl::remove_all_extents<T>;
				using type = T;
				remove_all_extents(void) = delete;
		}; // struct remove_all_extents

		template <typename T>
		struct remove_all_extents<T[]> final {
				using self = ca::impl::remove_all_extents<T[]>;
				using type = typename ca::impl::remove_all_extents<T>::type;
				remove_all_extents(void) = delete;
		}; // struct remove_all_extents

		template <typename T, ca::usz N>
		struct remove_all_extents<T[N]> final {
				using self = ca::impl::remove_all_extents<T[N]>;
				using type = typename ca::impl::remove_all_extents<T>::type;
				remove_all_extents(void) = delete;
		}; // struct remove_all_extents

	} // namespace impl


	/* @DOC std::remove_all_extents_t. */
	template <typename T>
	using remove_all_extents = typename ca::impl::remove_all_extents<T>::type;

} // namespace ca

#endif // ca_primitives_traits_remove_all_extents_hpp
