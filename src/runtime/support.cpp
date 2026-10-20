/* What compiled code and libca.a's remaining libc paths expect from libc, for programs linked
   without it (see start_x86_64.cpp): the stack protector's failure, errno (ca's own last error:
   one error code per thread, not two), and thread keys (ca::sys::impl's single-thread table,
   thread/impl/key_table). */

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/usz.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/thread/impl/key_table.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

extern "C" {

	[[noreturn, gnu::cold]] auto __stack_chk_fail(void) noexcept -> void { // ns-leak: ignore
		__builtin_trap();
	}

	auto __errno_location(void) noexcept -> int* { // ns-leak: ignore
		return &ca::sys::impl::last_code;
	}

	auto pthread_key_create(unsigned* const key, void (*)(void*)) noexcept -> int { // ns-leak: ignore

		const auto made = ca::sys::impl::create_key();
		if (!made) [[unlikely]]
			return made.error().code();
		*key = static_cast<unsigned>(*made);
		return 0;
	}

	auto pthread_setspecific(const unsigned key, const void* const value) noexcept -> int { // ns-leak: ignore

		const auto done = ca::sys::impl::set_key(key, const_cast<void*>(value));
		return done ? 0 : done.error().code();
	}

	auto pthread_getspecific(const unsigned key) noexcept -> void* { // ns-leak: ignore
		return ca::sys::impl::key_value(key);
	}

} // extern "C"

#endif
