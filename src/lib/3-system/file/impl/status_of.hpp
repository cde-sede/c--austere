#ifndef ca_system_file_impl_status_of_hpp
#define ca_system_file_impl_status_of_hpp

#include "3-system/file/file_status.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- STATUS OF ------------------------------------------------------

		namespace impl {

			[[nodiscard]] auto status_of(const void* raw) noexcept -> ca::sys::file_status;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_file_impl_status_of_hpp
