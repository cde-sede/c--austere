#ifndef ca_system_thread_futex_wake_hpp
#define ca_system_thread_futex_wake_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FUTEX WAKE -----------------------------------------------------

		/* @DOC Wakes up to count threads sleeping in futex_wait on word; ca::limits<u32>::max() wakes all.
		   Always a syscall: skip it when no thread can be waiting. */
		auto futex_wake(const ca::atomic<ca::u32>& word, ca::u32 count) noexcept -> void;

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_futex_wake_hpp
