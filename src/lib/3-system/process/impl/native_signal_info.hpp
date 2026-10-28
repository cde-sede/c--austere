#ifndef ca_system_process_impl_native_signal_info_hpp
#define ca_system_process_impl_native_signal_info_hpp

#include "3-system/process/signal_info.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- NATIVE SIGNAL INFO ---------------------------------------------

		namespace impl {

			/* @DOC The signal_info of siginfo_t native. */
			[[nodiscard]] auto from_native_info(const void* native) noexcept -> ca::sys::signal_info;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_process_impl_native_signal_info_hpp
