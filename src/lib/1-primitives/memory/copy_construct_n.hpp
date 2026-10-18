#ifndef ca_primitives_memory_copy_construct_n_hpp
#define ca_primitives_memory_copy_construct_n_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_trivially_copyable.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- COPY CONSTRUCT N ---------------------------------------------------

	/* @DOC Copy-constructs count objects from `from` into uninitialized storage at `to`. Ranges
	   must not overlap. Trivially copyable T: one memcpy. */
	template <typename T>
	requires ca::is_copy_constructible<T>
	auto copy_construct_n(const T* const from, const ca::usz count, T* const to) noexcept -> void {
		if constexpr (ca::is_trivially_copyable<T>) {
			if (count != 0U)
				__builtin_memcpy(static_cast<void*>(to), static_cast<const void*>(from), count * sizeof(T));
		}
		else {
			for (ca::usz i = 0U; i < count; ++i)
				ca::construct_at(to + i, from[i]);
		}
	}

} // namespace ca

#endif // ca_primitives_memory_copy_construct_n_hpp
