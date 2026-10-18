#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/thread/thread_key_set.hpp"

#include <pthread.h>


[[nodiscard]] auto ca::sys::thread_key_set(const ca::sys::thread_key key, void* const value) noexcept -> ca::result<void> {

	const int code = ::pthread_setspecific(static_cast<pthread_key_t>(key.id()), value);
	if (code != 0) [[unlikely]]
		return ca::failure{ca::error{code}};
	return ca::result<void>{};
}
