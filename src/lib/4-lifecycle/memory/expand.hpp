#ifndef ca_lifecycle_memory_expand_hpp
#define ca_lifecycle_memory_expand_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- EXPAND -------------------------------------------------------------

	namespace impl {

		template <typename A>
		concept has_expand = requires(const A& allocator, const ca::allocation block, const ca::usz size, const ca::usz alignment) {
			{ allocator.expand(block, size, alignment) } -> ca::is_same<ca::optional<ca::allocation>>;
		};

	} // namespace impl


	/* @DOC Resizes block to at least size bytes without moving it: allocator.expand when it has one,
	   else block itself when size fits in it. nullopt when it cannot, block then untouched. */
	template <ca::is_allocator A>
	[[nodiscard]] constexpr auto expand(const A& allocator, const ca::allocation block, const ca::usz size, const ca::usz alignment) noexcept
	-> ca::optional<ca::allocation> {
		if constexpr (ca::impl::has_expand<A>)
			return allocator.expand(block, size, alignment);
		else {
			if (size <= block.size())
				return block;
			return ca::nullopt;
		}
	}

} // namespace ca

#endif // ca_lifecycle_memory_expand_hpp
