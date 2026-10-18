#ifndef ca_primitives_memory_is_trivially_relocatable_hpp
#define ca_primitives_memory_is_trivially_relocatable_hpp

#include "1-primitives/traits/is_trivially_copyable.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS TRIVIALLY RELOCATABLE -------------------------------------------

	namespace impl {

		template <typename T>
		concept has_trivial_relocation_opt_in = requires {
			requires T::TRIVIALLY_RELOCATABLE;
		};

	} // namespace impl

	/* @DOC Types a byte copy can move to new storage, the source then dropped without its
	   destructor: trivially copyable types, and types declaring
	   `static constexpr bool TRIVIALLY_RELOCATABLE = true` (no self-pointers, no registration by
	   address). Opt-in, unlike C++26 std::is_trivially_relocatable. */
	template <typename T>
	concept is_trivially_relocatable = ca::is_trivially_copyable<T> || ca::impl::has_trivial_relocation_opt_in<T>;

} // namespace ca

#endif // ca_primitives_memory_is_trivially_relocatable_hpp
