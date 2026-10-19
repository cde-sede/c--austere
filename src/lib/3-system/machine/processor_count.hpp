#ifndef ca_system_machine_processor_count_hpp
#define ca_system_machine_processor_count_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- PROCESSOR COUNT ------------------------------------------------

		/* @DOC Processors this process may run on now (its affinity mask, as nproc counts them on
		   Linux; the online ones elsewhere), at least 1. Never fails. */
		[[nodiscard]] auto processor_count(void) noexcept -> ca::usz;

	} // namespace sys

} // namespace ca

#endif // ca_system_machine_processor_count_hpp
