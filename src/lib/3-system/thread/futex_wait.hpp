#ifndef ca_system_thread_futex_wait_hpp
#define ca_system_thread_futex_wait_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FUTEX WAIT -----------------------------------------------------

		/* @DOC Sleeps while word holds expected, until a futex_wake on it. Returns at once when it
		   holds something else, and may return early (a signal, a stale wake): callers read word
		   again in a loop. Process-private words only. Outside linux it yields once instead. */
		auto futex_wait(const ca::atomic<ca::u32>& word, ca::u32 expected) noexcept -> void;

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_futex_wait_hpp
