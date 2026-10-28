/* fork, execvp, posix_spawn, posix_spawnp, the posix_spawn_file_actions_* functions and the
   posix_spawnattr_* ones ca uses (flags: POSIX_SPAWN_SETPGROUP only) for programs linked
   without libc (see start_x86_64.cpp): ca::sys::impl's (process/impl/spawn), their errors as
   libc reports them. */

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/environment/environ.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/impl/spawn.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

static auto list_of(void* const storage) noexcept -> ca::sys::impl::spawn_list& {
	return *static_cast<ca::sys::impl::spawn_list*>(storage);
}

/* 0, or the error's code: what posix_spawn and its file actions return. */
static auto code_of(const ca::result<void>& done) noexcept -> int {
	return done ? 0 : done.error().code();
}

static auto attributes_of(void* const storage) noexcept -> ca::sys::impl::spawn_attributes& {
	return *static_cast<ca::sys::impl::spawn_attributes*>(storage);
}

static auto spawn(int* const pid, const char* const path, const bool search, const void* const actions, const void* const attributes, char* const* const argv, char* const* const envp) noexcept -> int {

	const auto child = ca::sys::impl::spawn_process(path, search, static_cast<const ca::sys::impl::spawn_list*>(actions), static_cast<const ca::sys::impl::spawn_attributes*>(attributes), argv, envp);
	if (!child) [[unlikely]]
		return child.error().code();
	if (pid != nullptr)
		*pid = *child;
	return 0;
}

extern "C" {

	auto fork(void) noexcept -> int { // ns-leak: ignore

		const auto child = ca::sys::impl::fork_process();
		return child ? *child : -1;
	}

	auto execvp(const char* const file, char* const* const argv) noexcept -> int { // ns-leak: ignore

		ca::sys::set_last_error(ca::sys::impl::execute_searching(file, argv, ca::sys::environ()));
		return -1;
	}

	auto posix_spawn(int* const pid, const char* const path, const void* const actions, const void* const attributes, char* const* const argv, char* const* const envp) noexcept -> int { // ns-leak: ignore
		return spawn(pid, path, false, actions, attributes, argv, envp);
	}

	auto posix_spawnp(int* const pid, const char* const file, const void* const actions, const void* const attributes, char* const* const argv, char* const* const envp) noexcept -> int { // ns-leak: ignore
		return spawn(pid, file, true, actions, attributes, argv, envp);
	}

	auto posix_spawnattr_init(void* const storage) noexcept -> int { // ns-leak: ignore

		attributes_of(storage) = ca::sys::impl::spawn_attributes{0, 0};
		return 0;
	}

	auto posix_spawnattr_destroy(void*) noexcept -> int { // ns-leak: ignore
		return 0;
	}

	auto posix_spawnattr_setflags(void* const storage, const short flags) noexcept -> int { // ns-leak: ignore

		if ((flags & ~ca::sys::impl::spawn_sets_process_group) != 0) [[unlikely]]
			return ca::errc::invalid_argument.code();
		attributes_of(storage)._flags = flags;
		return 0;
	}

	auto posix_spawnattr_setpgroup(void* const storage, const int group) noexcept -> int { // ns-leak: ignore

		attributes_of(storage)._process_group = group;
		return 0;
	}

	auto posix_spawn_file_actions_init(void* const storage) noexcept -> int { // ns-leak: ignore

		ca::sys::impl::spawn_list_init(list_of(storage));
		return 0;
	}

	auto posix_spawn_file_actions_destroy(void* const storage) noexcept -> int { // ns-leak: ignore

		ca::sys::impl::spawn_list_destroy(list_of(storage));
		return 0;
	}

	auto posix_spawn_file_actions_addclose(void* const storage, const int fd) noexcept -> int { // ns-leak: ignore
		return code_of(ca::sys::impl::spawn_list_add_close(list_of(storage), fd));
	}

	auto posix_spawn_file_actions_adddup2(void* const storage, const int fd, const int target) noexcept -> int { // ns-leak: ignore
		return code_of(ca::sys::impl::spawn_list_add_dup2(list_of(storage), fd, target));
	}

	auto posix_spawn_file_actions_addopen(void* const storage, const int fd, const char* const path, const int flags, const ca::u32 mode) noexcept -> int { // ns-leak: ignore
		return code_of(ca::sys::impl::spawn_list_add_open(list_of(storage), fd, path, flags, mode));
	}

} // extern "C"

#endif
