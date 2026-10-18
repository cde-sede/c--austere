#ifndef ca_primitives_memory_destroy_at_hpp
#define ca_primitives_memory_destroy_at_hpp

#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/utility/addressof.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DESTROY AT ---------------------------------------------------------

	/* @DOC Ends the lifetime of *address; arrays element by element. */
	template <typename T>
	constexpr auto destroy_at(T* address) noexcept -> void {

		if constexpr (ca::is_array<T>) {
			for (auto& element : *address)
				ca::destroy_at(ca::addressof(element));
		}
		else
			address->~T();
	}

} // namespace ca

#endif // ca_primitives_memory_destroy_at_hpp
