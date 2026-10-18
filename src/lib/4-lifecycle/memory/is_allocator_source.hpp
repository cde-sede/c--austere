#ifndef ca_lifecycle_memory_is_allocator_source_hpp
#define ca_lifecycle_memory_is_allocator_source_hpp

#include "1-primitives/traits/remove_cvref.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_allocator_resource.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS ALLOCATOR SOURCE ------------------------------------------------

	/* @DOC Where ca::make_in and ca::as_allocator take memory from: an allocator, or a resource
	   handing one out, references and cv-qualifiers aside. */
	template <typename R>
	concept is_allocator_source = ca::is_allocator<ca::remove_cvref<R>> || ca::is_allocator_resource<ca::remove_cvref<R>>;

} // namespace ca

#endif // ca_lifecycle_memory_is_allocator_source_hpp
