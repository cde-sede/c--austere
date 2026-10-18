#ifndef ca_lifecycle_memory_reallocate_hpp
#define ca_lifecycle_memory_reallocate_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/numeric/min.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/expand.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REALLOCATE ---------------------------------------------------------

	namespace impl {

		template <typename A>
		concept has_reallocate = requires(const A& allocator, const ca::allocation block, const ca::usz size, const ca::usz alignment) {
			{ allocator.reallocate(block, size, alignment) } -> ca::is_same<ca::result<ca::allocation>>;
		};

	} // namespace impl


	/* @DOC Resizes block to at least size bytes, keeping its bytes up to the smaller size; may move
	   it. allocator.reallocate when it has one, else ca::expand, else allocate, copy and
	   deallocate. On failure block is untouched. */
	template <ca::is_allocator A>
	[[nodiscard]] auto reallocate(const A& allocator, const ca::allocation block, const ca::usz size, const ca::usz alignment) noexcept
	-> ca::result<ca::allocation> {
		if constexpr (ca::impl::has_reallocate<A>)
			return allocator.reallocate(block, size, alignment);
		else {
			auto expanded = ca::expand(allocator, block, size, alignment);
			if (expanded)
				return *expanded;
			auto moved = allocator.allocate(size, alignment);
			if (!moved) [[unlikely]]
				return moved;
			ca::copy_bytes(moved->data(), block.data(), ca::min(block.size(), size));
			allocator.deallocate(block, alignment);
			return moved;
		}
	}

} // namespace ca

#endif // ca_lifecycle_memory_reallocate_hpp
