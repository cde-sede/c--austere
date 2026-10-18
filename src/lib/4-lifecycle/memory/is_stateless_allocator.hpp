#ifndef ca_lifecycle_memory_is_stateless_allocator_hpp
#define ca_lifecycle_memory_is_stateless_allocator_hpp

#include "4-lifecycle/memory/is_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS STATELESS ALLOCATOR ---------------------------------------------

	/* @DOC Allocators whose instances are interchangeable: memory from one goes back through any
	   other, a default-constructed one included. Opt-in, static constexpr bool STATELESS = true;
	   an empty class is not enough. Allocator-aware types store them in no space. */
	template <typename A>
	concept is_stateless_allocator = ca::is_allocator<A> && ca::impl::has_stateless_claim<A>;

} // namespace ca

#endif // ca_lifecycle_memory_is_stateless_allocator_hpp
