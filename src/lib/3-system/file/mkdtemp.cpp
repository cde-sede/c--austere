#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/impl/temporary_name.hpp"
#include "3-system/file/mkdir.hpp"
#include "3-system/file/mkdtemp.hpp"

#include <stdlib.h>
#include <unistd.h>


[[nodiscard]] auto ca::sys::mkdtemp(char* const pattern) noexcept -> ca::result<void> {

#if ca_platform(linux) && ca_architecture(x86_64)
	const auto suffix = ca::sys::impl::temporary_suffix(pattern);
	if (!suffix) [[unlikely]]
		return suffix.propagate();
	for (int attempt = 0; attempt < ca::sys::impl::TEMPORARY_ATTEMPTS; ++attempt) {
		const auto named = ca::sys::impl::randomize_suffix(*suffix);
		if (!named) [[unlikely]]
			return named;
		auto made = ca::sys::mkdir(pattern, 0700U);
		if (made || made.error() != ca::errc::exists)
			return made;
	}
	return ca::failure{ca::errc::exists};
#else
	if (::mkdtemp(pattern) == nullptr) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
#endif
}
