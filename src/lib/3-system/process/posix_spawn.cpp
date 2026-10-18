#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/environment/environ.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/process/posix_spawn.hpp"

#include <spawn.h>
#include <sys/types.h>


[[nodiscard]] auto ca::sys::posix_spawn(const char* const path, char* const* const argv, char* const* const envp, const ca::sys::spawn_actions* const actions) noexcept -> ca::result<ca::i32> {

	if (actions != nullptr && !actions->is_live()) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	pid_t child{-1};
	const int code = ::posix_spawn(&child, path, reinterpret_cast<const posix_spawn_file_actions_t*>(actions), nullptr, argv,
		envp != nullptr ? envp : ca::sys::environ());
	if (code != 0) [[unlikely]]
		return ca::failure{ca::error{code}};
	return static_cast<ca::i32>(child);
}
