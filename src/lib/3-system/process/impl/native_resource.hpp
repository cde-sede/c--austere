#ifndef ca_system_process_impl_native_resource_hpp
#define ca_system_process_impl_native_resource_hpp

#include "3-system/process/resource.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- NATIVE RESOURCE ------------------------------------------------

		namespace impl {

			/* @DOC The RLIMIT_* constant of which. */
			[[nodiscard]] auto native_resource(ca::sys::resource which) noexcept -> int;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_process_impl_native_resource_hpp
