#ifndef ca_primitives_atomic_atomic_signal_fence_hpp
#define ca_primitives_atomic_atomic_signal_fence_hpp

#include "1-primitives/atomic/memory_order.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ATOMIC SIGNAL FENCE ------------------------------------------------

	/* @DOC Compiler fence between a thread and its signal handlers, order M. */
	template <ca::memory_order M = ca::memory_order::seq_cst>
	auto atomic_signal_fence(void) noexcept -> void {
		__atomic_signal_fence(static_cast<int>(M));
	}

} // namespace ca

#endif // ca_primitives_atomic_atomic_signal_fence_hpp
