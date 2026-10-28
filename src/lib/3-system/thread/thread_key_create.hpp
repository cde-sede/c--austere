#ifndef ca_system_thread_thread_key_create_hpp
#define ca_system_thread_thread_key_create_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/thread/exit_stage.hpp"
#include "3-system/thread/thread_key.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- THREAD KEY CREATE ----------------------------------------------

		/* @DOC A new per-thread slot, null in every thread. When a thread other than the main one
		   exits with a non-null value there, on_exit(value) runs in it, the slot cleared first,
		   in the order of stage. errc::try_again once 32 exist. */
		[[nodiscard]] auto thread_key_create(void (*on_exit)(void*), ca::sys::exit_stage stage = ca::sys::exit_stage::user) noexcept -> ca::result<ca::sys::thread_key>;

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_thread_key_create_hpp
