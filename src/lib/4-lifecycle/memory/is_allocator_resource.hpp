#ifndef ca_lifecycle_memory_is_allocator_resource_hpp
#define ca_lifecycle_memory_is_allocator_resource_hpp

#include "4-lifecycle/memory/is_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS ALLOCATOR RESOURCE ----------------------------------------------

	/* @DOC Owners of memory (arena) that are not allocators themselves and give out stateful
	   allocators through allocator(). The resource outlives every allocator it gave out. */
	template <typename R>
	concept is_allocator_resource = !ca::is_allocator<R> && requires(R& resource) {
		{ resource.allocator() } -> ca::is_allocator;
	};

} // namespace ca

#endif // ca_lifecycle_memory_is_allocator_resource_hpp
