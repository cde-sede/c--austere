#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/thread/thread_key_create.hpp"

#include <pthread.h>


static_assert(sizeof(pthread_key_t) <= sizeof(ca::usz));

[[nodiscard]] auto ca::sys::thread_key_create(void (* const on_exit)(void*)) noexcept -> ca::result<ca::sys::thread_key> {

	pthread_key_t key{};
	const int code = ::pthread_key_create(&key, on_exit);
	if (code != 0) [[unlikely]]
		return ca::failure{ca::error{code}};
	return ca::sys::thread_key{static_cast<ca::usz>(key)};
}
