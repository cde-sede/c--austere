// expect: exit 0
// link: libc
#include "1-primitives/atomic/atomic.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/time/clock.hpp"
#include "3-system/time/now.hpp"
#include "3-system/time/sleep_for.hpp"
#include "5-collections/concurrent/channel.hpp"

#include <pthread.h>

static constexpr int threads = 3;
static constexpr long per_thread = 100000;
static constexpr long total = threads * per_thread;

static ca::channel<long>* shared = nullptr;
static ca::atomic<long> sum{0};
static ca::atomic<long> received{0};

static auto produce(void* const argument) -> void* {

	const long first = static_cast<long>(reinterpret_cast<__INTPTR_TYPE__>(argument)) * per_thread;
	for (long i = first; i < first + per_thread; ++i) {
		if (!shared->push(i))
			__builtin_trap();
	}
	return nullptr;
}

static auto consume(void*) -> void* {

	long local = 0;
	long count = 0;
	for (;;) {
		auto value = shared->pop();
		if (!value) {
			if (value.error() != ca::errc::broken_pipe)
				__builtin_trap();
			break;
		}
		local += *value;
		++count;
	}
	sum.fetch_add(local);
	received.fetch_add(count);
	return nullptr;
}

static auto wait_then_close(void*) -> void* {

	auto value = shared->pop();
	if (value || value.error() != ca::errc::broken_pipe)
		__builtin_trap();
	return nullptr;
}

auto main(void) -> int {
	auto small = ca::channel<int>::make(2U);
	if (!small || small->capacity() != 2U || small->closed())
		return 1;
	if (!small->push(1) || !small->try_push(2))
		return 2;
	auto full = small->try_push(3);
	if (full || full.error() != ca::errc::no_space)
		return 3;
	const ca::i64 start = ca::sys::now(ca::sys::clock::monotonic);
	auto late = small->push_for(3, 20000000);
	if (late || late.error() != ca::errc::timed_out || ca::sys::now(ca::sys::clock::monotonic) - start < 20000000)
		return 4;
	auto first = small->pop();
	auto second = small->try_pop();
	if (!first || *first != 1 || !second || *second != 2 || small->try_pop())
		return 5;
	auto idle = small->pop_for(1000000);
	if (idle || idle.error() != ca::errc::timed_out)
		return 6;
	if (!small->push(4))
		return 7;
	small->close();
	small->close();
	auto refused = small->push(5);
	auto refused_now = small->try_push(5);
	if (!small->closed() || refused || refused.error() != ca::errc::broken_pipe || refused_now || refused_now.error() != ca::errc::broken_pipe)
		return 8;
	auto drained = small->pop();
	auto ended = small->pop_until(0);
	if (!drained || *drained != 4 || ended || ended.error() != ca::errc::broken_pipe)
		return 9;
	if (!small->cleanup() || small->capacity() != 0U)
		return 10;

	auto sleeping = ca::channel<long>::make(4U);
	if (!sleeping)
		return 11;
	shared = &*sleeping;
	pthread_t waiter;
	if (pthread_create(&waiter, nullptr, wait_then_close, nullptr) != 0)
		return 12;
	if (!ca::sys::sleep_for(20000000))
		return 12;
	shared->close();
	pthread_join(waiter, nullptr);
	if (!sleeping->cleanup())
		return 13;

	auto made = ca::channel<long>::make(8U);
	if (!made)
		return 14;
	shared = &*made;
	pthread_t producers[threads];
	pthread_t consumers[threads];
	for (int t = 0; t < threads; ++t) {
		if (pthread_create(&producers[t], nullptr, produce, reinterpret_cast<void*>(static_cast<__INTPTR_TYPE__>(t))) != 0)
			return 15;
		if (pthread_create(&consumers[t], nullptr, consume, nullptr) != 0)
			return 15;
	}
	for (pthread_t& producer : producers)
		pthread_join(producer, nullptr);
	shared->close();
	for (pthread_t& consumer : consumers)
		pthread_join(consumer, nullptr);
	if (received.load() != total || sum.load() != total * (total - 1) / 2)
		return 16;
	return made->cleanup() ? 0 : 17;
}
