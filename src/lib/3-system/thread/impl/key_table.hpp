#ifndef ca_system_thread_impl_key_table_hpp
#define ca_system_thread_impl_key_table_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- KEY TABLE ------------------------------------------------------

		/* @DOC Thread keys without libc, for a process of one thread (ca starts none yet): one
		   value per key, process-wide. What libca_runtime's pthread_key_create, setspecific and
		   getspecific are; ca::sys::thread_key_create and thread_key_set call those. As glibc's
		   for the main thread, destructors never run. Linux x86-64 only. */
		namespace impl {

			/* @DOC A new key, its value null. errc::try_again once 128 exist. */
			[[nodiscard]] auto create_key(void) noexcept -> ca::result<ca::usz>;

			/* @DOC Sets key's value. errc::invalid_argument for a key never created. */
			[[nodiscard]] auto set_key(ca::usz key, void* value) noexcept -> ca::result<void>;

			/* @DOC key's value; null for a key never created. */
			[[nodiscard]] auto key_value(ca::usz key) noexcept -> void*;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_impl_key_table_hpp
