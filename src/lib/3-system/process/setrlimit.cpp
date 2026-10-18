#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/impl/native_resource.hpp"
#include "3-system/process/setrlimit.hpp"

#include <sys/resource.h>


static auto native_bound(const ca::u64 value) noexcept -> rlim_t {
	return value == ca::sys::resource_limit::UNLIMITED ? RLIM_INFINITY : static_cast<rlim_t>(value);
}

[[nodiscard]] auto ca::sys::setrlimit(const ca::sys::resource which, const ca::sys::resource_limit& limit) noexcept -> ca::result<void> {

	const rlimit raw{native_bound(limit.current()), native_bound(limit.maximum())};
	if (::setrlimit(ca::sys::impl::native_resource(which), &raw) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::result<void>{};
}
