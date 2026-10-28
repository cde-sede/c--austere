// expect: exit 0
// link: libc
#include "3-system/thread/thread_key_create.hpp"
#include "3-system/thread/thread_key_set.hpp"

#include <pthread.h>

static char order[8];
static int ran = 0;
static ca::sys::thread_key keys[4]{ca::sys::thread_key{0U}, ca::sys::thread_key{0U}, ca::sys::thread_key{0U}, ca::sys::thread_key{0U}};
static char names[4] = {'m', 'b', 'u', 'v'};

static auto record(void* const name) noexcept -> void {

	order[ran++] = *static_cast<char*>(name);
	if (*static_cast<char*>(name) == 'b')
		static_cast<void>(ca::sys::thread_key_set(keys[2], &names[2]));
}

static auto worker(void*) -> void* {

	for (int i = 0; i < 4; ++i) {
		if (!ca::sys::thread_key_set(keys[i], &names[i]))
			return &ran;
	}
	return nullptr;
}

auto main(void) -> int {
	auto memory = ca::sys::thread_key_create(&record, ca::sys::exit_stage::memory);
	auto buffers = ca::sys::thread_key_create(&record, ca::sys::exit_stage::buffers);
	auto user = ca::sys::thread_key_create(&record);
	auto newer = ca::sys::thread_key_create(&record, ca::sys::exit_stage::user);
	if (!memory || !buffers || !user || !newer)
		return 1;
	keys[0] = *memory;
	keys[1] = *buffers;
	keys[2] = *user;
	keys[3] = *newer;
	if (ca::sys::thread_key_set(ca::sys::thread_key{31U}, nullptr))
		return 2;
	pthread_t thread{};
	void* status = &thread;
	if (pthread_create(&thread, nullptr, &worker, nullptr) != 0 || pthread_join(thread, &status) != 0 || status != nullptr)
		return 3;
	return ran == 5 && order[0] == 'v' && order[1] == 'u' && order[2] == 'b' && order[3] == 'm' && order[4] == 'u' ? 0 : 4;
}
