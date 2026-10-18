/* @DOC Placement operator new tagged with ca::impl::placement, so it cannot collide with <new>. The
   one declaration outside namespace ca; use ca::construct_at. */

#ifndef ca_primitives_memory_placement_new_hpp
#define ca_primitives_memory_placement_new_hpp

#include "1-primitives/debug/assume.hpp"
#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- PLACEMENT NEW ------------------------------------------------------

	namespace impl {

		struct placement final {
				using self = ca::impl::placement;
		}; // struct placement

	} // namespace impl

} // namespace ca


// -- PLACEMENT NEW ----------------------------------------------------------

[[nodiscard, gnu::returns_nonnull]] inline auto operator new(ca::usz, void* address, ca::impl::placement) noexcept -> void* { // ns-leak: ignore

	ca::assume(address != nullptr);
	return address;
}

#endif // ca_primitives_memory_placement_new_hpp
