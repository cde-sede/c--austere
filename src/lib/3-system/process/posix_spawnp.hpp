#ifndef ca_system_process_posix_spawnp_hpp
#define ca_system_process_posix_spawnp_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/process/spawn_actions.hpp"
#include "3-system/process/spawn_options.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- POSIX SPAWNP ---------------------------------------------------

		/* @DOC Starts program file, searched in PATH when it has no slash, as a child with arguments argv (null-terminated), environment envp
		   (null-terminated; nullptr inherits ours), actions (nullptr for none) and options (its
		   process group); returns its id. Cheaper than fork then exec, and safe in a multithreaded
		   parent. */
		[[nodiscard]] auto posix_spawnp(const char* file, char* const* argv, char* const* envp, const ca::sys::spawn_actions* actions, const ca::sys::spawn_options& options = ca::sys::spawn_options()) noexcept -> ca::result<ca::i32>;

	} // namespace sys

} // namespace ca

#endif // ca_system_process_posix_spawnp_hpp
