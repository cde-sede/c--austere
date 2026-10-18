#ifndef ca_lifecycle_memory_allocate_n_hpp
#define ca_lifecycle_memory_allocate_n_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/numeric/mul_overflow.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ALLOCATE N ---------------------------------------------------------

	/* @DOC Uninitialized room for count T, aligned for T; size() / sizeof(T) of them are usable.
	   errc::out_of_memory when count * sizeof(T) overflows. */
	template <typename T, ca::is_allocator A>
	[[nodiscard, gnu::always_inline]] inline auto allocate_n(const A& allocator, const ca::usz count) noexcept -> ca::result<ca::allocation> {

		ca::usz size{0U};
		if (ca::mul_overflow(size, count, sizeof(T))) [[unlikely]]
			return ca::failure{ca::errc::out_of_memory};
		return allocator.allocate(size, alignof(T));
	}

} // namespace ca

#endif // ca_lifecycle_memory_allocate_n_hpp
