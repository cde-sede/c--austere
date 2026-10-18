#ifndef ca_lifecycle_memory_allocate_zeroed_hpp
#define ca_lifecycle_memory_allocate_zeroed_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/fill_bytes.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ALLOCATE ZEROED ----------------------------------------------------

	namespace impl {

		template <typename A>
		concept has_allocate_zeroed = requires(const A& allocator, const ca::usz size, const ca::usz alignment) {
			{ allocator.allocate_zeroed(size, alignment) } -> ca::is_same<ca::result<ca::allocation>>;
		};

	} // namespace impl


	/* @DOC allocate, with every usable byte zero: allocator.allocate_zeroed when it has one (fresh
	   pages skip the fill), else allocate and fill. */
	template <ca::is_allocator A>
	[[nodiscard]] auto allocate_zeroed(const A& allocator, const ca::usz size, const ca::usz alignment) noexcept -> ca::result<ca::allocation> {
		if constexpr (ca::impl::has_allocate_zeroed<A>)
			return allocator.allocate_zeroed(size, alignment);
		else {
			auto block = allocator.allocate(size, alignment);
			if (block) [[likely]]
				ca::fill_bytes(block->data(), 0U, block->size());
			return block;
		}
	}

} // namespace ca

#endif // ca_lifecycle_memory_allocate_zeroed_hpp
