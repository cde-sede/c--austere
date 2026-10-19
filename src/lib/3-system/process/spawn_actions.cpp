#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/process/spawn_actions.hpp"

#include <spawn.h>
#include <sys/types.h>


namespace ca {


	namespace sys {


		namespace impl {

			struct spawn_actions_layout final {
					using self = ca::sys::impl::spawn_actions_layout;
					static_assert(sizeof(ca::sys::spawn_actions::_storage) == sizeof(::posix_spawn_file_actions_t));
					static_assert(alignof(void*) == alignof(::posix_spawn_file_actions_t));
					static_assert(__builtin_offsetof(ca::sys::spawn_actions, _storage) == 0U);
			}; // struct spawn_actions_layout

		} // namespace impl

	} // namespace sys

} // namespace ca

static auto native(ca::byte* const storage) noexcept -> posix_spawn_file_actions_t* {
	return reinterpret_cast<posix_spawn_file_actions_t*>(storage);
}

[[nodiscard]] static auto check(const int code) noexcept -> ca::result<void> {

	if (code != 0) [[unlikely]]
		return ca::failure{ca::error{code}};
	return ca::result<void>{};
}

// ca::sys::spawn_actions::~spawn_actions(void) noexcept {
// 	if (_live)
// 		static_cast<void>(this->cleanup());
// }

[[nodiscard]] auto ca::sys::spawn_actions::init(void) noexcept -> ca::result<void> {

	if (_live) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	auto done = check(::posix_spawn_file_actions_init(native(_storage)));
	_live = static_cast<bool>(done);
	return done;
}

[[nodiscard]] auto ca::sys::spawn_actions::add_dup2(const int from, const int to) noexcept -> ca::result<void> {

	if (!_live) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	return check(::posix_spawn_file_actions_adddup2(native(_storage), from, to));
}

[[nodiscard]] auto ca::sys::spawn_actions::add_close(const int fd) noexcept -> ca::result<void> {

	if (!_live) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	return check(::posix_spawn_file_actions_addclose(native(_storage), fd));
}

[[nodiscard]] auto ca::sys::spawn_actions::add_open(const int fd, const char* const path, const int flags, const ca::u32 mode) noexcept -> ca::result<void> {

	if (!_live) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	return check(::posix_spawn_file_actions_addopen(native(_storage), fd, path, flags, static_cast<mode_t>(mode)));
}

[[nodiscard]] auto ca::sys::spawn_actions::cleanup(void) noexcept -> ca::result<void> {

	if (!_live) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	_live = false;
	return check(::posix_spawn_file_actions_destroy(native(_storage)));
}
