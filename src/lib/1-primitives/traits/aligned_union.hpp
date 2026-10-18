#ifndef ca_primitives_traits_aligned_union_hpp
#define ca_primitives_traits_aligned_union_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/aligned_storage.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ALIGNED UNION ------------------------------------------------------

	namespace impl {

		template <ca::usz... Ns>
		inline constexpr ca::usz max_of = 0U;

		template <ca::usz N, ca::usz... Ns>
		inline constexpr ca::usz max_of<N, Ns...> = (N > ca::impl::max_of<Ns...>) ? N : ca::impl::max_of<Ns...>;

	} // namespace impl


	/* @DOC std::aligned_union_t. Deprecated in std, kept for parity. No alignment_value member. */
	template <ca::usz L, typename T, typename... Ts>
	using aligned_union = ca::aligned_storage<ca::impl::max_of<L, sizeof(T), sizeof(Ts)...>,
		ca::impl::max_of<alignof(T), alignof(Ts)...>>;

} // namespace ca

#endif // ca_primitives_traits_aligned_union_hpp
