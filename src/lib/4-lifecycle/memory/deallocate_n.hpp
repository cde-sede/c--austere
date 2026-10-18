#ifndef ca_lifecycle_memory_deallocate_n_hpp
#define ca_lifecycle_memory_deallocate_n_hpp

#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DEALLOCATE N -------------------------------------------------------

	/* @DOC Hands back a block from allocate_n<T> or a resize of it. */
	template <typename T, ca::is_allocator A>
	[[gnu::always_inline]] inline auto deallocate_n(const A& allocator, const ca::allocation block) noexcept -> void {
		allocator.deallocate(block, alignof(T));
	}

} // namespace ca

#endif // ca_lifecycle_memory_deallocate_n_hpp
