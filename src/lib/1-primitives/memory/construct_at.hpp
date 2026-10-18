#ifndef ca_primitives_memory_construct_at_hpp
#define ca_primitives_memory_construct_at_hpp

#include "1-primitives/memory/placement_new.hpp"
#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/utility/forward.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- CONSTRUCT AT -------------------------------------------------------

	/* @DOC Constructs a T at address from args; returns it. Not usable in constant evaluation
	   (C++20). */
	template <typename T, typename... Args>
	requires ca::is_constructible<T, Args...>
	constexpr auto construct_at(T* address, Args&&... args) noexcept -> T* {
		return ::new (const_cast<void*>(static_cast<const volatile void*>(address)), ca::impl::placement{})
			T(ca::forward<Args>(args)...);
	}

} // namespace ca

#endif // ca_primitives_memory_construct_at_hpp
