#ifndef ca_system_process_posix_spawn_hpp
#define ca_system_process_posix_spawn_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/process/spawn_actions.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- POSIX SPAWN ----------------------------------------------------

		/* @DOC Starts program path as a child with arguments argv (null-terminated), environment envp
		   (null-terminated; nullptr inherits ours) and actions (nullptr for none); returns its id. Cheaper
		   than fork then exec, and safe in a multithreaded parent. */
		[[nodiscard]] auto posix_spawn(const char* path, char* const* argv, char* const* envp, const ca::sys::spawn_actions* actions) noexcept -> ca::result<ca::i32>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_posix_spawn_hpp
