/* environ, getenv, setenv and unsetenv for programs linked without libc (see start_x86_64.cpp,
   which points environ at the envp the kernel passed): ca::sys::impl's
   (environment/impl/variables), errors in ca's last error, which is errno here. */

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/environment/impl/variables.hpp"
#include "3-system/error/last_error.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

/* 0, or -1 with the error as errno: what setenv and unsetenv return. */
static auto status_of(const ca::result<void>& done) noexcept -> int {

	if (done)
		return 0;
	ca::sys::set_last_error(done.error());
	return -1;
}

extern "C" {

	char** environ = nullptr; // ns-leak: ignore

	auto getenv(const char* const name) noexcept -> char* { // ns-leak: ignore
		return ca::sys::impl::find_variable(name);
	}

	auto setenv(const char* const name, const char* const value, const int overwrite) noexcept -> int { // ns-leak: ignore
		return status_of(ca::sys::impl::set_variable(name, value, overwrite != 0));
	}

	auto unsetenv(const char* const name) noexcept -> int { // ns-leak: ignore
		return status_of(ca::sys::impl::unset_variable(name));
	}

} // extern "C"

#endif
