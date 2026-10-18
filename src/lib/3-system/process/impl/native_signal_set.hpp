#ifndef ca_system_process_impl_native_signal_set_hpp
#define ca_system_process_impl_native_signal_set_hpp

#include "3-system/process/signal_set.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- NATIVE SIGNAL SET ----------------------------------------------

		namespace impl {

			/* @DOC Fills sigset_t native from set. */
			auto to_native(const ca::sys::signal_set& set, void* native) noexcept -> void;

			/* @DOC The signals 1..64 in sigset_t native. */
			[[nodiscard]] auto from_native(const void* native) noexcept -> ca::sys::signal_set;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_process_impl_native_signal_set_hpp
