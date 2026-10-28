#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/environment/environ.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/process/impl/spawn.hpp"
#include "3-system/process/impl/start_child.hpp"

#include <spawn.h>
#include <sys/types.h>


static_assert(ca::sys::impl::spawn_sets_process_group == POSIX_SPAWN_SETPGROUP);
static_assert(sizeof(ca::sys::impl::spawn_attributes) <= sizeof(::posix_spawnattr_t));
static_assert(alignof(ca::sys::impl::spawn_attributes) <= alignof(::posix_spawnattr_t));

[[nodiscard]] static auto spawn(pid_t* const child, const char* const path, const bool search, char* const* const argv, char* const* const envp, const posix_spawn_file_actions_t* const actions, const posix_spawnattr_t* const attributes) noexcept -> int {

	if (search)
		return ::posix_spawnp(child, path, actions, attributes, argv, envp);
	return ::posix_spawn(child, path, actions, attributes, argv, envp);
}

[[nodiscard]] auto ca::sys::impl::start_child(const char* const path, const bool search, char* const* const argv, char* const* const envp, const ca::sys::spawn_actions* const actions, const ca::sys::spawn_options& options) noexcept -> ca::result<ca::i32> {

	if (actions != nullptr && !actions->is_live()) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	const auto* const native_actions = reinterpret_cast<const posix_spawn_file_actions_t*>(actions);
	char* const* const environment = envp != nullptr ? envp : ca::sys::environ();
	pid_t child{-1};
	const auto group = options.process_group();
	if (!group) {
		const int code = spawn(&child, path, search, argv, environment, native_actions, nullptr);
		if (code != 0) [[unlikely]]
			return ca::failure{ca::error{code}};
		return static_cast<ca::i32>(child);
	}
	posix_spawnattr_t attributes;
	int code = ::posix_spawnattr_init(&attributes);
	if (code != 0) [[unlikely]]
		return ca::failure{ca::error{code}};
	code = ::posix_spawnattr_setflags(&attributes, POSIX_SPAWN_SETPGROUP);
	if (code == 0)
		code = ::posix_spawnattr_setpgroup(&attributes, static_cast<pid_t>(*group));
	if (code == 0)
		code = spawn(&child, path, search, argv, environment, native_actions, &attributes);
	static_cast<void>(::posix_spawnattr_destroy(&attributes));
	if (code != 0) [[unlikely]]
		return ca::failure{ca::error{code}};
	return static_cast<ca::i32>(child);
}
