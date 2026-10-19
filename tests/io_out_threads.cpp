// expect: exit 0
// link: libc
#include "3-system/file/read.hpp"
#include "3-system/io/dup2.hpp"
#include "3-system/io/pipe.hpp"
#include "6-resources/io/out.hpp"
#include "6-resources/print/flush.hpp"
#include "6-resources/print/print.hpp"

#include <pthread.h>

static auto worker(void* const raw) -> void* {

	auto printed = ca::print("thread {} ", reinterpret_cast<ca::usz>(raw));
	return printed && *printed == 9U && ca::io::out().pending() != 0U ? nullptr : raw;
}

auto main(void) -> int {
	auto ends = ca::sys::pipe(0);
	if (!ends || !ca::sys::dup2(ends->write_end(), 1))
		return 1;
	if (!ca::print("main "))
		return 6;
	pthread_t thread{};
	void* status = &thread;
	if (pthread_create(&thread, nullptr, &worker, reinterpret_cast<void*>(7)) != 0 || pthread_join(thread, &status) != 0 || status != nullptr)
		return 2;
	char got[64];
	auto first = ca::sys::read(ends->read_end(), got);
	if (!first || ca::sv::str{got, *first} != ca::sv::str{"thread 7 "})
		return 3;
	if (!ca::flush())
		return 4;
	auto second = ca::sys::read(ends->read_end(), got);
	return second && ca::sv::str{got, *second} == ca::sv::str{"main "} ? 0 : 5;
}
