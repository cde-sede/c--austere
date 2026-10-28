#include "3-system/process/impl/start_child.hpp"
#include "3-system/process/posix_spawnp.hpp"


[[nodiscard]] auto ca::sys::posix_spawnp(const char* const file, char* const* const argv, char* const* const envp, const ca::sys::spawn_actions* const actions, const ca::sys::spawn_options& options) noexcept -> ca::result<ca::i32> {
	return ca::sys::impl::start_child(file, true, argv, envp, actions, options);
}
