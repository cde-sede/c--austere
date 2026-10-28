#include "3-system/process/impl/start_child.hpp"
#include "3-system/process/posix_spawn.hpp"


[[nodiscard]] auto ca::sys::posix_spawn(const char* const path, char* const* const argv, char* const* const envp, const ca::sys::spawn_actions* const actions, const ca::sys::spawn_options& options) noexcept -> ca::result<ca::i32> {
	return ca::sys::impl::start_child(path, false, argv, envp, actions, options);
}
