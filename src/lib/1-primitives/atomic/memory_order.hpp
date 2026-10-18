#ifndef ca_primitives_atomic_memory_order_hpp
#define ca_primitives_atomic_memory_order_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MEMORY ORDER -------------------------------------------------------

	/* @DOC Orderings of the __atomic builtins. */
	enum class memory_order : int {
		relaxed = __ATOMIC_RELAXED,
		consume = __ATOMIC_CONSUME,
		acquire = __ATOMIC_ACQUIRE,
		release = __ATOMIC_RELEASE,
		acq_rel = __ATOMIC_ACQ_REL,
		seq_cst = __ATOMIC_SEQ_CST,
	};

} // namespace ca

#endif // ca_primitives_atomic_memory_order_hpp
