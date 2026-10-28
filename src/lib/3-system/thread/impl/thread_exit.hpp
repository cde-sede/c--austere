#ifndef ca_system_thread_impl_thread_exit_hpp
#define ca_system_thread_impl_thread_exit_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- THREAD EXIT ----------------------------------------------------

		/* @DOC What a thread runs as it ends, without libc: the destructors of its thread_local
		   objects (what libca_runtime's __cxa_thread_atexit registers), then its thread key
		   destructors. Lists are per thread, grown with sys::reallocarray. Linux x86-64 only. */
		namespace impl {

			/* @DOC Registers function(object) with the calling thread. */
			[[nodiscard]] auto at_thread_exit(void (*function)(void*), void* object) noexcept -> ca::result<void>;

			/* @DOC The calling thread's registrations in reverse, those they make too; the list is
			   freed. What exit runs for the thread calling it. */
			auto run_thread_exit_handlers(void) noexcept -> void;

			/* @DOC run_thread_exit_handlers, then run_key_destructors: the end of a thread ca
			   started, before it leaves the kernel. */
			auto finish_thread(void) noexcept -> void;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_impl_thread_exit_hpp
