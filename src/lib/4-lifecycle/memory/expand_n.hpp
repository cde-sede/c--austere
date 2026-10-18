#ifndef ca_lifecycle_memory_expand_n_hpp
#define ca_lifecycle_memory_expand_n_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/numeric/mul_overflow.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/expand.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- EXPAND N -----------------------------------------------------------

	/* @DOC ca::expand to room for count T; nullopt also when count * sizeof(T) overflows. */
	template <typename T, ca::is_allocator A>
	[[nodiscard]] auto expand_n(const A& allocator, const ca::allocation block, const ca::usz count) noexcept -> ca::optional<ca::allocation> {

		ca::usz size{0U};
		if (ca::mul_overflow(size, count, sizeof(T))) [[unlikely]]
			return ca::nullopt;
		return ca::expand(allocator, block, size, alignof(T));
	}

} // namespace ca

#endif // ca_lifecycle_memory_expand_n_hpp
