#ifndef ca_lifecycle_memory_as_allocator_hpp
#define ca_lifecycle_memory_as_allocator_hpp

#include "1-primitives/traits/remove_cvref.hpp"
#include "4-lifecycle/memory/allocator_of.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_allocator_source.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- AS ALLOCATOR -------------------------------------------------------

	/* @DOC A copy of source when it is an allocator, source.allocator() when it is a resource. */
	template <ca::is_allocator_source R>
	[[nodiscard]] constexpr auto as_allocator(R&& source) noexcept -> ca::allocator_of<R> {

		if constexpr (ca::is_allocator<ca::remove_cvref<R>>)
			return source;
		else
			return source.allocator();
	}

} // namespace ca

#endif // ca_lifecycle_memory_as_allocator_hpp
