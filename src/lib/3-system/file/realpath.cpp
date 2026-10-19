#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/file_kind.hpp"
#include "3-system/file/getcwd.hpp"
#include "3-system/file/lstat.hpp"
#include "3-system/file/readlink.hpp"
#include "3-system/file/realpath.hpp"
#include "3-system/memory/mallocarray.hpp"

#include <stdlib.h>


#if ca_platform(linux) && ca_architecture(x86_64)

static constexpr ca::usz LIMIT = 4096U;
static constexpr int LINKS = 40;

static auto fail(const ca::error error) noexcept -> ca::result<char*> {

	ca::sys::set_last_error(error);
	return ca::failure{error};
}

/* Resolves path as the kernel would, without libc: components left to right on a resolved
   prefix, "." dropped, ".." back to the parent, each symlink's target put in front of what
   remains (40 at most, as the kernel), every component required to exist. */
[[nodiscard]] auto ca::sys::realpath(const char* const path) noexcept -> ca::result<char*> {

	char resolved[LIMIT];
	char rest[LIMIT];
	char swap[LIMIT];
	ca::usz length = 0U;
	ca::usz left = 0U;
	for (; path[left] != '\0'; ++left) {
		if (left + 1U >= LIMIT) [[unlikely]]
			return fail(ca::errc::name_too_long);
		rest[left] = path[left];
	}
	rest[left] = '\0';
	if (left == 0U) [[unlikely]]
		return fail(ca::errc::no_entry);
	if (rest[0] != '/') {
		const auto cwd = ca::sys::getcwd(ca::span<char>{resolved, LIMIT});
		if (!cwd) [[unlikely]]
			return cwd.propagate();
		length = *cwd == 1U ? 0U : *cwd;
	}
	ca::usz at = 0U;
	int links = 0;
	for (;;) {
		while (rest[at] == '/')
			++at;
		if (rest[at] == '\0')
			break;
		ca::usz end = at;
		while (rest[end] != '/' && rest[end] != '\0')
			++end;
		const ca::usz size = end - at;
		if (size == 1U && rest[at] == '.') {
			at = end;
			continue;
		}
		if (size == 2U && rest[at] == '.' && rest[at + 1U] == '.') {
			while (length > 0U && resolved[length - 1U] != '/')
				--length;
			if (length > 0U)
				--length;
			at = end;
			continue;
		}
		const ca::usz before = length;
		if (length + 1U + size + 1U > LIMIT) [[unlikely]]
			return fail(ca::errc::name_too_long);
		resolved[length++] = '/';
		for (ca::usz k = 0U; k < size; ++k)
			resolved[length++] = rest[at + k];
		resolved[length] = '\0';
		const auto status = ca::sys::lstat(resolved);
		if (!status) [[unlikely]]
			return status.propagate();
		at = end;
		if (status->kind() == ca::sys::file_kind::symlink) {
			if (++links > LINKS) [[unlikely]]
				return fail(ca::errc::symlink_loop);
			const auto target = ca::sys::readlink(resolved, ca::span<char>{swap, LIMIT});
			if (!target) [[unlikely]]
				return target.propagate();
			ca::usz copied = *target;
			if (copied >= LIMIT - 1U) [[unlikely]]
				return fail(ca::errc::name_too_long);
			if (rest[at] != '\0')
				swap[copied++] = '/';
			for (ca::usz k = at; rest[k] != '\0'; ++k) {
				if (copied + 1U >= LIMIT) [[unlikely]]
					return fail(ca::errc::name_too_long);
				swap[copied++] = rest[k];
			}
			for (ca::usz k = 0U; k < copied; ++k)
				rest[k] = swap[k];
			rest[copied] = '\0';
			at = 0U;
			length = rest[0] == '/' ? 0U : before;
			continue;
		}
		bool more = false;
		for (ca::usz k = at; rest[k] != '\0' && !more; ++k)
			more = rest[k] == '/';
		if (more && status->kind() != ca::sys::file_kind::directory) [[unlikely]]
			return fail(ca::errc::not_a_directory);
	}
	if (length == 0U)
		resolved[length++] = '/';
	const auto block = ca::sys::mallocarray(length + 1U, 1U);
	if (!block) [[unlikely]]
		return block.propagate();
	char* const out = static_cast<char*>(*block);
	for (ca::usz k = 0U; k < length; ++k)
		out[k] = resolved[k];
	out[length] = '\0';
	return out;
}

#else

[[nodiscard]] auto ca::sys::realpath(const char* const path) noexcept -> ca::result<char*> {

	char* const resolved = ::realpath(path, nullptr);
	if (resolved == nullptr) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return resolved;
}

#endif
