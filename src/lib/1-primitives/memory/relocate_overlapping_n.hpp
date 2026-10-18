#ifndef ca_primitives_memory_relocate_overlapping_n_hpp
#define ca_primitives_memory_relocate_overlapping_n_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/utility/move.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- RELOCATE OVERLAPPING N ---------------------------------------------

	/* @DOC relocate_n within one array: the ranges may overlap. Slots of `to` outside `from` must
	   be uninitialized; slots of `from` outside `to` end up so. Trivially relocatable T: one
	   memmove. */
	template <typename T>
	auto relocate_overlapping_n(T* const from, const ca::usz count, T* const to) noexcept -> void {
		if constexpr (ca::is_trivially_relocatable<T>) {
			if (count != 0U)
				__builtin_memmove(static_cast<void*>(to), static_cast<const void*>(from), count * sizeof(T));
		}
		else if (to < from) {
			for (ca::usz i = 0U; i < count; ++i) {
				ca::construct_at(to + i, ca::move(from[i]));
				ca::destroy_at(from + i);
			}
		}
		else {
			for (ca::usz i = count; i != 0U; --i) {
				ca::construct_at(to + i - 1U, ca::move(from[i - 1U]));
				ca::destroy_at(from + i - 1U);
			}
		}
	}

} // namespace ca

#endif // ca_primitives_memory_relocate_overlapping_n_hpp
