#ifndef ca_system_thread_impl_ordered_keys_hpp
#define ca_system_thread_impl_ordered_keys_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/thread/exit_stage.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- ORDERED KEYS ---------------------------------------------------

		/* @DOC ca's own thread keys: 32 slots, values in thread_local storage, callbacks run by
		   stage from the destructor of one system key, which a thread sets the first time one of
		   its values becomes non-null. What thread_key_create and thread_key_set are. */
		namespace impl {

			[[nodiscard]] auto create_ordered_key(void (*on_exit)(void*), ca::sys::exit_stage stage) noexcept -> ca::result<ca::usz>;

			[[nodiscard]] auto set_ordered_key(ca::usz key, void* value) noexcept -> ca::result<void>;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_impl_ordered_keys_hpp
