#ifndef ca_primitives_traits_aligned_storage_hpp
#define ca_primitives_traits_aligned_storage_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/max_alignment.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ALIGNED STORAGE ----------------------------------------------------

	namespace impl {

		template <ca::usz L, ca::usz A>
		struct aligned_storage final {
				using self = ca::impl::aligned_storage<L, A>;
				struct type final {
						using self = typename ca::impl::aligned_storage<L, A>::type;
						alignas(A) unsigned char _data[L];
				}; // struct type
				aligned_storage(void) = delete;
		}; // struct aligned_storage

	} // namespace impl


	/* @DOC std::aligned_storage_t. Deprecated in std, kept for parity. Default alignment:
	   ca::max_alignment. */
	template <ca::usz L, ca::usz A = ca::max_alignment>
	using aligned_storage = typename ca::impl::aligned_storage<L, A>::type;

} // namespace ca

#endif // ca_primitives_traits_aligned_storage_hpp
