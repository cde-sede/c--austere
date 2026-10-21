#ifndef ca_system_time_impl_deadline_after_hpp
#define ca_system_time_impl_deadline_after_hpp

#include "1-primitives/integers/i64.hpp"
#include "1-primitives/limits.hpp"
#include "3-system/time/clock.hpp"
#include "3-system/time/now.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- DEADLINE AFTER -------------------------------------------------

		namespace impl {

			/* @DOC The monotonic deadline nanoseconds from now, saturated; now for a negative
			   count. What every *_for of the sync primitives passes to its *_until. */
			[[nodiscard]] inline auto deadline_after(const ca::i64 nanoseconds) noexcept -> ca::i64 {

				const ca::i64 start = ca::sys::now(ca::sys::clock::monotonic);
				if (nanoseconds <= 0)
					return start;
				return nanoseconds > ca::limits<ca::i64>::max() - start ? ca::limits<ca::i64>::max() : start + nanoseconds;
			}

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_time_impl_deadline_after_hpp
