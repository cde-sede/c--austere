#ifndef ca_primitives_traits_remove_extent_hpp
#define ca_primitives_traits_remove_extent_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REMOVE EXTENT ------------------------------------------------------

	namespace impl {

		template <typename T>
		struct remove_extent final {
				using self = ca::impl::remove_extent<T>;
				using type = T;
				remove_extent(void) = delete;
		}; // struct remove_extent

		template <typename T>
		struct remove_extent<T[]> final {
				using self = ca::impl::remove_extent<T[]>;
				using type = T;
				remove_extent(void) = delete;
		}; // struct remove_extent

		template <typename T, ca::usz N>
		struct remove_extent<T[N]> final {
				using self = ca::impl::remove_extent<T[N]>;
				using type = T;
				remove_extent(void) = delete;
		}; // struct remove_extent

	} // namespace impl


	/* @DOC std::remove_extent_t. */
	template <typename T>
	using remove_extent = typename ca::impl::remove_extent<T>::type;

} // namespace ca

#endif // ca_primitives_traits_remove_extent_hpp
