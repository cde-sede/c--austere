#ifndef ca_lifecycle_memory_reallocate_n_hpp
#define ca_lifecycle_memory_reallocate_n_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/numeric/mul_overflow.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/reallocate.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REALLOCATE N -------------------------------------------------------

	/* @DOC ca::reallocate to room for count T. Moves bytes, so T must be trivially relocatable.
	   errc::out_of_memory when count * sizeof(T) overflows; block then untouched. */
	template <typename T, ca::is_allocator A>
	requires ca::is_trivially_relocatable<T>
	[[nodiscard]] auto reallocate_n(const A& allocator, const ca::allocation block, const ca::usz count) noexcept -> ca::result<ca::allocation> {

		ca::usz size{0U};
		if (ca::mul_overflow(size, count, sizeof(T))) [[unlikely]]
			return ca::failure{ca::errc::out_of_memory};
		return ca::reallocate(allocator, block, size, alignof(T));
	}

} // namespace ca

#endif // ca_lifecycle_memory_reallocate_n_hpp
