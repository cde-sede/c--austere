#ifndef ca_lifecycle_memory_is_allocator_aware_hpp
#define ca_lifecycle_memory_is_allocator_aware_hpp

#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS ALLOCATOR AWARE -------------------------------------------------

	/* @DOC Types drawing all their memory from an allocator they name: allocator_type, and
	   rebind<B>, the same type over allocator B. ca::make_in builds them over any allocator, from
	   a static make_in(allocator, args...) -> result<T> when they have one, else from a
	   constructor taking the allocator first (after error& when it reports). */
	template <typename T>
	concept is_allocator_aware = ca::is_allocator<typename T::allocator_type> && requires {
		typename T::template rebind<ca::heap>;
	};

} // namespace ca

#endif // ca_lifecycle_memory_is_allocator_aware_hpp
