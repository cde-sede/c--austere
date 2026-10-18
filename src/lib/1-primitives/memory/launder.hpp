#ifndef ca_primitives_memory_launder_hpp
#define ca_primitives_memory_launder_hpp

#include "1-primitives/traits/is_function.hpp"
#include "1-primitives/traits/is_void.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- LAUNDER ------------------------------------------------------------

	/* @DOC Pointer to the object now living at the address of pointer. */
	template <typename T>
	requires (!ca::is_function<T> && !ca::is_void<T>)
	[[nodiscard]] constexpr auto launder(T* pointer) noexcept -> T* {
		return __builtin_launder(pointer);
	}

} // namespace ca

#endif // ca_primitives_memory_launder_hpp
