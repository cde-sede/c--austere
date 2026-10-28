#ifndef ca_system_thread_impl_key_table_hpp
#define ca_system_thread_impl_key_table_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- KEY TABLE ------------------------------------------------------

		/* @DOC Thread keys without libc: what libca_runtime's pthread_key_create, setspecific
		   and getspecific are. Values per thread, in thread_local storage; destructors run when a
		   thread ca started exits (run_key_destructors), never for the main thread, as glibc.
		   Linux x86-64 only. */
		namespace impl {

			/* @DOC A new key, its value null in every thread. errc::try_again once 128 exist. */
			[[nodiscard]] auto create_key(void (*destructor)(void*)) noexcept -> ca::result<ca::usz>;

			/* @DOC Sets key's value in the calling thread. errc::invalid_argument for a key never
			   created. */
			[[nodiscard]] auto set_key(ca::usz key, void* value) noexcept -> ca::result<void>;

			/* @DOC key's value in the calling thread; null for a key never created. */
			[[nodiscard]] auto key_value(ca::usz key) noexcept -> void*;

			/* @DOC The calling thread's destructors, in key order, of every non-null value (cleared
			   first), again while they set values: four passes at most (PTHREAD_DESTRUCTOR_ITERATIONS). */
			auto run_key_destructors(void) noexcept -> void;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_impl_key_table_hpp
