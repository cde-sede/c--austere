#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/string_length.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/environment/impl/variables.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/memory/mmap_anonymous.hpp"
#include "3-system/memory/protection.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

#include <unistd.h>

static constexpr ca::usz CHUNK = 65536U;

static char* arena = nullptr; // file-state: ignore
static ca::usz arena_left = 0U; // file-state: ignore

/* size bytes that stay allocated, from 64 KiB mappings (a larger one alone); null when out of
   memory. */
static auto keep(const ca::usz size) noexcept -> char* {

	const ca::usz rounded = (size + 15U) & ~ca::usz{15U};
	if (rounded > arena_left) {
		const ca::usz length = rounded > CHUNK ? rounded : CHUNK;
		const auto mapped = ca::sys::mmap_anonymous(length, ca::sys::protection::read | ca::sys::protection::write);
		if (!mapped) [[unlikely]]
			return nullptr;
		char* const fresh = reinterpret_cast<char*>(mapped->data());
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

/* The value of entry when it is "name=...", else null: compared byte by byte, as entry may be
   shorter than name. */
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

[[nodiscard]] auto ca::sys::impl::find_variable(const char* const name) noexcept -> char* {

	for (char** entry = ::environ; entry != nullptr && *entry != nullptr; ++entry) {
		const char* const value = value_of(*entry, name);
		if (value != nullptr)
			return const_cast<char*>(value);
	}
	return nullptr;
}

[[nodiscard]] auto ca::sys::impl::set_variable(const char* const name, const char* const value, const bool overwrite) noexcept -> ca::result<void> {

	if (!valid(name)) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	ca::usz count = 0U;
	char** found = nullptr;
	for (char** entry = ::environ; entry != nullptr && *entry != nullptr; ++entry, ++count) {
		if (found == nullptr && value_of(*entry, name) != nullptr)
			found = entry;
	}
	if (found != nullptr && !overwrite)
		return ca::result<void>{};
	const ca::usz name_length = ca::string_length(name);
	const ca::usz value_length = ca::string_length(value);
	char* const text = keep(name_length + value_length + 2U);
	if (text == nullptr) [[unlikely]]
		return ca::failure{ca::errc::out_of_memory};
	ca::copy_bytes(text, name, name_length);
	text[name_length] = '=';
	ca::copy_bytes(text + name_length + 1U, value, value_length + 1U);
	if (found != nullptr) {
		*found = text;
		return ca::result<void>{};
	}
	char** const grown = reinterpret_cast<char**>(keep((count + 2U) * sizeof(char*)));
	if (grown == nullptr) [[unlikely]]
		return ca::failure{ca::errc::out_of_memory};
	ca::copy_bytes(grown, ::environ, count * sizeof(char*));
	     grown[count] = text;
	grown[count + 1U] = nullptr;
	        ::environ = grown;
	return ca::result<void>{};
}

[[nodiscard]] auto ca::sys::impl::unset_variable(const char* const name) noexcept -> ca::result<void> {

	if (!valid(name)) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	if (::environ == nullptr)
		return ca::result<void>{};
	char** kept = ::environ;
	for (char** entry = ::environ; *entry != nullptr; ++entry) {
		if (value_of(*entry, name) == nullptr)
			*kept++ = *entry;
	}
	*kept = nullptr;
	return ca::result<void>{};
}

#endif
