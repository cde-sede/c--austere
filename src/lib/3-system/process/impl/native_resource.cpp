#include "3-system/process/impl/native_resource.hpp"

#include <sys/resource.h>


[[nodiscard]] auto ca::sys::impl::native_resource(const ca::sys::resource which) noexcept -> int {
	switch (which) {
		case ca::sys::resource::address_space:
			return RLIMIT_AS;
		case ca::sys::resource::core_size:
			return RLIMIT_CORE;
		case ca::sys::resource::cpu_time:
			return RLIMIT_CPU;
		case ca::sys::resource::data_size:
			return RLIMIT_DATA;
		case ca::sys::resource::file_size:
			return RLIMIT_FSIZE;
		case ca::sys::resource::open_files:
			return RLIMIT_NOFILE;
		case ca::sys::resource::processes:
			return RLIMIT_NPROC;
		default:
			return RLIMIT_STACK;
	}
}
