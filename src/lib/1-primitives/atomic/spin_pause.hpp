#ifndef ca_primitives_atomic_spin_pause_hpp
#define ca_primitives_atomic_spin_pause_hpp

#include "1-primitives/config/architecture.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SPIN PAUSE ---------------------------------------------------------

	/* @DOC Tells the CPU the thread is spinning on a value another thread will change: pause on
	   x86-64, yield on aarch64, nothing elsewhere. Neither a fence nor a syscall. */
	[[gnu::always_inline]] inline auto spin_pause(void) noexcept -> void {

#if ca_architecture(x86_64)
		__builtin_ia32_pause();
#elif ca_architecture(aarch64)
		asm volatile("yield" : : : "memory");
#endif
	}

} // namespace ca

#endif // ca_primitives_atomic_spin_pause_hpp
