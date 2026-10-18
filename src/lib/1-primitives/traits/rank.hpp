#ifndef ca_primitives_traits_rank_hpp
#define ca_primitives_traits_rank_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- RANK ---------------------------------------------------------------

	namespace impl {

		template <typename T>
		inline constexpr ca::usz rank = 0U;

		template <typename T>
		inline constexpr ca::usz rank<T[]> = ca::impl::rank<T> + 1U;

		template <typename T, ca::usz N>
		inline constexpr ca::usz rank<T[N]> = ca::impl::rank<T> + 1U;

	} // namespace impl


	/* @DOC std::rank_v. */
	template <typename T>
	inline constexpr ca::usz rank = ca::impl::rank<T>;

} // namespace ca

#endif // ca_primitives_traits_rank_hpp
