#ifndef ca_primitives_memory_relocate_n_hpp
#define ca_primitives_memory_relocate_n_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/utility/move.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- RELOCATE N ---------------------------------------------------------

	/* @DOC Moves count objects from `from` into uninitialized storage at `to`, ending the lifetime
	   of the sources. Ranges must not overlap. Trivially relocatable T: one memcpy. */
	template <typename T>
	auto relocate_n(T* const from, const ca::usz count, T* const to) noexcept -> void {
		if constexpr (ca::is_trivially_relocatable<T>) {
			if (count != 0U)
				__builtin_memcpy(static_cast<void*>(to), static_cast<const void*>(from), count * sizeof(T));
		}
		else {
			for (ca::usz i = 0U; i < count; ++i) {
				ca::construct_at(to + i, ca::move(from[i]));
				ca::destroy_at(from + i);
			}
		}
	}

} // namespace ca

#endif // ca_primitives_memory_relocate_n_hpp
