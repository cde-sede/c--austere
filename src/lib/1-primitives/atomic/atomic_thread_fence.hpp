#ifndef ca_primitives_atomic_atomic_thread_fence_hpp
#define ca_primitives_atomic_atomic_thread_fence_hpp

#include "1-primitives/atomic/memory_order.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ATOMIC THREAD FENCE ------------------------------------------------

	/* @DOC Memory fence between threads, order M. */
	template <ca::memory_order M = ca::memory_order::seq_cst>
	auto atomic_thread_fence(void) noexcept -> void {
		__atomic_thread_fence(static_cast<int>(M));
	}

} // namespace ca

#endif // ca_primitives_atomic_atomic_thread_fence_hpp
