// expect: exit 0
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/chdir.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <pthread.h>

static auto other_thread(void*) -> void* {

	if (ca::sys::last_error() != ca::error{0})
		return reinterpret_cast<void*>(1);
	static_cast<void>(ca::syscall::close(-1));
	return ca::sys::last_error() == ca::errc::bad_descriptor ? nullptr : reinterpret_cast<void*>(2);
}

auto main(void) -> int {
	if (ca::sys::last_error() != ca::error{0})
		return 1;

	auto missing = ca::sys::chdir("/nonexistent/ca/last_error");
	if (missing || missing.error() != ca::errc::no_entry || ca::sys::last_error() != ca::errc::no_entry)
		return 2;

	pthread_t thread{};
	void* outcome = nullptr;
	if (::pthread_create(&thread, nullptr, other_thread, nullptr) != 0 || ::pthread_join(thread, &outcome) != 0 || outcome != nullptr)
		return 3;
	if (ca::sys::last_error() != ca::errc::no_entry)
		return 4;

	if (!ca::syscall::getpid() || ca::sys::last_error() != ca::errc::no_entry)
		return 5;
	return 0;
}
