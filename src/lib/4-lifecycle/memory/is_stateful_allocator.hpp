#ifndef ca_lifecycle_memory_is_stateful_allocator_hpp
#define ca_lifecycle_memory_is_stateful_allocator_hpp

#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS STATEFUL ALLOCATOR ----------------------------------------------

	/* @DOC Allocators carrying state, usually a pointer to a resource: memory goes back through the
	   allocator it came from or a copy. Allocator-aware types holding one have no default
	   constructor, and move the allocator with the memory. */
	template <typename A>
	concept is_stateful_allocator = ca::is_allocator<A> && !ca::is_stateless_allocator<A>;

} // namespace ca

#endif // ca_lifecycle_memory_is_stateful_allocator_hpp
