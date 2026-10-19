/* What compiled code and libca.a's remaining libc paths expect from libc, for programs linked
   without it (see start_x86_64.cpp): the stack protector's failure, errno, and thread keys for a
   single-threaded process (ca starts no thread yet; like glibc, key destructors do not run when
   the main thread exits). */

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/usz.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

static constexpr ca::usz KEYS = 128U;

static void* key_values[KEYS]; // file-state: ignore
static unsigned keys_made = 0U; // file-state: ignore
static constinit thread_local int error_number = 0; // file-state: ignore

extern "C" {

	[[noreturn, gnu::cold]] auto __stack_chk_fail(void) noexcept -> void { // ns-leak: ignore
		__builtin_trap();
	}

	auto __errno_location(void) noexcept -> int* { // ns-leak: ignore
		return &error_number;
	}

	auto pthread_key_create(unsigned* const key, void (*)(void*)) noexcept -> int { // ns-leak: ignore

		if (keys_made == KEYS) [[unlikely]]
			return 11;
		*key = keys_made++;
		return 0;
	}

	auto pthread_setspecific(const unsigned key, const void* const value) noexcept -> int { // ns-leak: ignore

		if (key >= keys_made) [[unlikely]]
			return 22;
		key_values[key] = const_cast<void*>(value);
		return 0;
	}

	auto pthread_getspecific(const unsigned key) noexcept -> void* { // ns-leak: ignore
		return key < keys_made ? key_values[key] : nullptr;
	}

} // extern "C"

#endif
