#ifndef ca_system_process_impl_wait_status_of_hpp
#define ca_system_process_impl_wait_status_of_hpp

#include "3-system/process/wait_status.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- WAIT STATUS OF -------------------------------------------------

		namespace impl {

			[[nodiscard]] auto wait_status_of(ca::i32 child, int raw) noexcept -> ca::sys::wait_status;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_process_impl_wait_status_of_hpp
