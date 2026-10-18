#ifndef ca_primitives_traits_is_bounded_array_hpp
#define ca_primitives_traits_is_bounded_array_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS BOUNDED ARRAY ---------------------------------------------------

	namespace impl {

		template <typename T>
		inline constexpr bool is_bounded_array = false;

		template <typename T, ca::usz N>
		inline constexpr bool is_bounded_array<T[N]> = true;

	} // namespace impl


	/* @DOC std::is_bounded_array_v, as a concept. */
	template <typename T>
	concept is_bounded_array = ca::impl::is_bounded_array<T>;

} // namespace ca

#endif // ca_primitives_traits_is_bounded_array_hpp
