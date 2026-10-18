#ifndef ca_primitives_memory_destroy_n_hpp
#define ca_primitives_memory_destroy_n_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/traits/is_trivially_destructible.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DESTROY N ----------------------------------------------------------

	/* @DOC Ends the lifetime of count objects starting at first, in order. */
	template <typename T>
	constexpr auto destroy_n(T* const first, const ca::usz count) noexcept -> void {
		if constexpr (!ca::is_trivially_destructible<T>) {
			for (ca::usz i = 0U; i < count; ++i)
				ca::destroy_at(first + i);
		}
	}

} // namespace ca

#endif // ca_primitives_memory_destroy_n_hpp
