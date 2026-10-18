#ifndef ca_system_thread_thread_key_set_hpp
#define ca_system_thread_thread_key_set_hpp

#include "2-vocabulary/result.hpp"
#include "3-system/thread/thread_key.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- THREAD KEY SET -------------------------------------------------

		/* @DOC Stores value in the calling thread's slot of key. */
		[[nodiscard]] auto thread_key_set(ca::sys::thread_key key, void* value) noexcept -> ca::result<void>;

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_thread_key_set_hpp
