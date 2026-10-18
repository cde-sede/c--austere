#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/getrusage.hpp"

#include <sys/resource.h>
#include <sys/time.h>


static auto nanoseconds_of(const timeval& time) noexcept -> ca::i64 {
	return static_cast<ca::i64>(time.tv_sec) * 1000000000 + static_cast<ca::i64>(time.tv_usec) * 1000;
}

[[nodiscard]] auto ca::sys::getrusage(const ca::sys::usage_target target) noexcept -> ca::result<ca::sys::resource_usage> {

	rusage raw{};
	if (::getrusage(target == ca::sys::usage_target::children ? RUSAGE_CHILDREN : RUSAGE_SELF, &raw) == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
#if ca_platform(linux)
	const ca::u64 resident = static_cast<ca::u64>(raw.ru_maxrss) * 1024U;
#else
	const ca::u64 resident = static_cast<ca::u64>(raw.ru_maxrss);
#endif
	return ca::sys::resource_usage{
		nanoseconds_of(raw.ru_utime),
		nanoseconds_of(raw.ru_stime),
		resident,
		static_cast<ca::u64>(raw.ru_minflt),
		static_cast<ca::u64>(raw.ru_majflt),
		static_cast<ca::u64>(raw.ru_nvcsw),
		static_cast<ca::u64>(raw.ru_nivcsw),
	};
}
