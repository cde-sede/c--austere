#include "1-primitives/atomic/atomic.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/sync/mutex.hpp"
#include "3-system/thread/impl/ordered_keys.hpp"

#include <pthread.h>


static constexpr ca::usz KEYS = 32U;
static constexpr int PASSES = 4;

struct ordered_key final { // ns-leak: ignore
		using self = ordered_key;
		void (*_on_exit)(void*);
		ca::sys::exit_stage _stage;
}; // struct ordered_key

static ordered_key keys[KEYS]; // file-state: ignore
static constinit ca::atomic<ca::usz> created{0U}; // file-state: ignore
static constinit ca::mutex creating; // file-state: ignore
static pthread_key_t dispatcher{}; // file-state: ignore
static constinit thread_local void* values[KEYS]{};
static constinit thread_local bool armed = false;

static auto run_stage(const ca::sys::exit_stage stage, const ca::usz count) noexcept -> bool {

	bool ran = false;
	for (ca::usz key = count; key-- != 0U;) {
		void* const value = values[key];
		if (value == nullptr || keys[key]._stage != stage)
			continue;
		values[key] = nullptr;
		keys[key]._on_exit(value);
		ran = true;
	}
	return ran;
}

/* armed stays set: values the callbacks store are seen by the next pass, not by a new dispatch. */
static auto dispatch(void*) noexcept -> void {

	armed = true;
	const ca::usz count = created.load<ca::memory_order::acquire>();
	for (int pass = 0; pass < PASSES; ++pass) {
		bool ran = run_stage(ca::sys::exit_stage::user, count);
		ran = run_stage(ca::sys::exit_stage::buffers, count) || ran;
		ran = run_stage(ca::sys::exit_stage::memory, count) || ran;
		if (!ran)
			return;
	}
}

[[nodiscard]] auto ca::sys::impl::create_ordered_key(void (* const on_exit)(void*), const ca::sys::exit_stage stage) noexcept -> ca::result<ca::usz> {

	creating.lock();
	const ca::usz count = created.load<ca::memory_order::relaxed>();
	int code = count == KEYS ? ca::errc::try_again.code() : 0;
	if (code == 0 && count == 0U)
		code = ::pthread_key_create(&dispatcher, &dispatch);
	if (code == 0) {
		keys[count] = ordered_key{on_exit, stage};
		created.store<ca::memory_order::release>(count + 1U);
	}
	creating.unlock();
	if (code != 0) [[unlikely]]
		return ca::failure{ca::error{code}};
	return count;
}

[[nodiscard]] auto ca::sys::impl::set_ordered_key(const ca::usz key, void* const value) noexcept -> ca::result<void> {

	if (key >= created.load<ca::memory_order::acquire>()) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	values[key] = value;
	if (value == nullptr || armed)
		return ca::result<void>{};
	const int code = ::pthread_setspecific(dispatcher, &armed);
	if (code != 0) [[unlikely]]
		return ca::failure{ca::error{code}};
	armed = true;
	return ca::result<void>{};
}
