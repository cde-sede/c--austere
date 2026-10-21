#ifndef ca_system_thread_futex_wait_until_hpp
#define ca_system_thread_futex_wait_until_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FUTEX WAIT UNTIL -----------------------------------------------

		/* @DOC futex_wait with a deadline on the monotonic clock (sys::now(clock::monotonic)
		   nanoseconds): errc::timed_out once it has passed, also when it already had. Early
		   returns are not failures; callers loop until the word changes or the deadline fails. */
		[[nodiscard]] auto futex_wait_until(const ca::atomic<ca::u32>& word, ca::u32 expected, ca::i64 deadline) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_futex_wait_until_hpp
