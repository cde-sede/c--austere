#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/getrlimit.hpp"
#include "3-system/process/impl/native_resource.hpp"

#include <sys/resource.h>


static auto bound_of(const rlim_t value) noexcept -> ca::u64 {
	return value == RLIM_INFINITY ? ca::sys::resource_limit::UNLIMITED : static_cast<ca::u64>(value);
}

[[nodiscard]] auto ca::sys::getrlimit(const ca::sys::resource which) noexcept -> ca::result<ca::sys::resource_limit> {

	rlimit raw{};
	if (::getrlimit(ca::sys::impl::native_resource(which), &raw) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return ca::sys::resource_limit{bound_of(raw.rlim_cur), bound_of(raw.rlim_max)};
}
