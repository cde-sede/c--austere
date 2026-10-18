#ifndef ca_system_machine_gethostname_hpp
#define ca_system_machine_gethostname_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- GETHOSTNAME ----------------------------------------------------

		/* @DOC Writes the host name into buffer, NUL-terminated; returns its length. errc::out_of_range when
		   it does not fit. */
		[[nodiscard]] auto gethostname(ca::span<char> buffer) noexcept -> ca::result<ca::usz>;

	} // namespace sys

} // namespace ca

#endif // ca_system_machine_gethostname_hpp
