#ifndef ca_system_process_impl_start_child_hpp
#define ca_system_process_impl_start_child_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/process/spawn_actions.hpp"
#include "3-system/process/spawn_options.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- START CHILD ----------------------------------------------------

		namespace impl {

			/* @DOC posix_spawn, or posix_spawnp when search, with actions (null for none) and options
			   as a posix_spawnattr_t made for the call; envp null inherits ours. */
			[[nodiscard]] auto start_child(const char* path, bool search, char* const* argv, char* const* envp, const ca::sys::spawn_actions* actions, const ca::sys::spawn_options& options) noexcept -> ca::result<ca::i32>;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_process_impl_start_child_hpp
