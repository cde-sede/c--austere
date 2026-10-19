/* environ, getenv, setenv and unsetenv for programs linked without libc (see start_x86_64.cpp,
   which points environ at the envp the kernel passed). setenv copies "NAME=value" into pages of
   its own and, to add a name, the array into a larger one; what it replaces stays allocated, as
   with glibc. Not thread-safe, as glibc's. */

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/isz.hpp"
#include "1-primitives/integers/usz.hpp"
#include "3-system/syscall/syscalls.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

extern "C" auto __errno_location(void) noexcept -> int*;

static constexpr ca::usz CHUNK = 65536U;

static char* arena = nullptr; // file-state: ignore
static ca::usz arena_left = 0U; // file-state: ignore

/* size bytes that stay allocated, from 64 KiB mappings (a larger one alone); null when out of
   memory. */
static auto keep(const ca::usz size) noexcept -> char* {

	const ca::usz rounded = (size + 15U) & ~ca::usz{15U};
	if (rounded > arena_left) {
		const ca::usz length = rounded > CHUNK ? (rounded + 4095U) & ~ca::usz{4095U} : CHUNK;
		const auto mapped = ca::syscall::mmap(nullptr, length, 3, 0x22, -1, 0);
		if (!mapped) [[unlikely]]
			return nullptr;
		char* const fresh = reinterpret_cast<char*>(*mapped);
		if (length != CHUNK)
			return fresh;
		arena = fresh;
		arena_left = CHUNK;
	}
	char* const block = arena;
	arena += rounded;
	arena_left -= rounded;
	return block;
}

static auto length_of(const char* const text) noexcept -> ca::usz {

	ca::usz length = 0U;
	while (text[length] != '\0')
		++length;
	return length;
}

/* The value of entry when it is "name=...", else null. */
static auto value_of(const char* const entry, const char* const name) noexcept -> const char* {

	ca::usz i = 0U;
	for (; name[i] != '\0'; ++i) {
		if (entry[i] != name[i])
			return nullptr;
	}
	return entry[i] == '=' ? entry + i + 1 : nullptr;
}

static auto valid(const char* const name) noexcept -> bool {

	if (name == nullptr || name[0] == '\0')
		return false;
	for (const char* at = name; *at != '\0'; ++at) {
		if (*at == '=')
			return false;
	}
	return true;
}

static auto fail(const int code) noexcept -> int {

	*__errno_location() = code;
	return -1;
}

extern "C" {

	char** environ = nullptr; // ns-leak: ignore

	auto getenv(const char* const name) noexcept -> char* { // ns-leak: ignore

		if (environ == nullptr)
			return nullptr;
		for (char** entry = environ; *entry != nullptr; ++entry) {
			const char* const value = value_of(*entry, name);
			if (value != nullptr)
				return const_cast<char*>(value);
		}
		return nullptr;
	}

	auto setenv(const char* const name, const char* const value, const int overwrite) noexcept -> int { // ns-leak: ignore

		if (!valid(name))
			return fail(22);
		ca::usz count = 0U;
		char** found = nullptr;
		for (char** entry = environ; entry != nullptr && *entry != nullptr; ++entry, ++count) {
			if (found == nullptr && value_of(*entry, name) != nullptr)
				found = entry;
		}
		if (found != nullptr && overwrite == 0)
			return 0;
		const ca::usz name_length = length_of(name);
		const ca::usz value_length = length_of(value);
		char* const text = keep(name_length + value_length + 2U);
		if (text == nullptr) [[unlikely]]
			return fail(12);
		for (ca::usz i = 0U; i < name_length; ++i)
			text[i] = name[i];
		text[name_length] = '=';
		for (ca::usz i = 0U; i <= value_length; ++i)
			text[name_length + 1U + i] = value[i];
		if (found != nullptr) {
			*found = text;
			return 0;
		}
		char** const grown = reinterpret_cast<char**>(keep((count + 2U) * sizeof(char*)));
		if (grown == nullptr) [[unlikely]]
			return fail(12);
		for (ca::usz i = 0U; i < count; ++i)
			grown[i] = environ[i];
		  grown[count] = text;
		grown[count + 1U] = nullptr;
		environ = grown;
		return 0;
	}

	auto unsetenv(const char* const name) noexcept -> int { // ns-leak: ignore

		if (!valid(name))
			return fail(22);
		if (environ == nullptr)
			return 0;
		char** kept = environ;
		for (char** entry = environ; *entry != nullptr; ++entry) {
			if (value_of(*entry, name) == nullptr)
				*kept++ = *entry;
		}
		*kept = nullptr;
		return 0;
	}

} // extern "C"

#endif
