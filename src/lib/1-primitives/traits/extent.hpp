#ifndef ca_primitives_traits_extent_hpp
#define ca_primitives_traits_extent_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- EXTENT -------------------------------------------------------------

	namespace impl {

		template <typename T, unsigned I>
		inline constexpr ca::usz extent = 0U;

		template <typename T, unsigned I>
		inline constexpr ca::usz extent<T[], I> = ca::impl::extent<T, I - 1U>;

		template <typename T, ca::usz N, unsigned I>
		inline constexpr ca::usz extent<T[N], I> = ca::impl::extent<T, I - 1U>;

		template <typename T>
		inline constexpr ca::usz extent<T[], 0U> = 0U;

		template <typename T, ca::usz N>
		inline constexpr ca::usz extent<T[N], 0U> = N;

	} // namespace impl


	/* @DOC std::extent_v. */
	template <typename T, unsigned I = 0U>
	inline constexpr ca::usz extent = ca::impl::extent<T, I>;

} // namespace ca

#endif // ca_primitives_traits_extent_hpp
