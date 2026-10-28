// expect: exit 0
// link: libc
#include "1-primitives/atomic/atomic.hpp"
#include "3-system/error/errc.hpp"
#include "5-collections/concurrent/mpmc_queue.hpp"

#include <pthread.h>

static constexpr int threads = 4;
static constexpr long per_thread = 200000;
static constexpr long total = threads * per_thread;

static ca::atomic<long> taken{0};
static ca::atomic<long> sum{0};
static unsigned char seen[total];

struct work final {
	ca::mpmc_queue<long>* _queue;
	long _first;
};

static auto produce(void* const argument) -> void* {

	const work& job = *static_cast<const work*>(argument);
	for (long i = job._first; i < job._first + per_thread; ++i) {
		while (!job._queue->try_push(i)) {
		}
	}
	return nullptr;
}

static auto consume(void* const argument) -> void* {

	auto& queue = *static_cast<ca::mpmc_queue<long>*>(argument);
	long local = 0;
	while (taken.load<ca::memory_order::relaxed>() < total) {
		auto value = queue.try_pop();
		if (!value)
			continue;
		++seen[*value];
		local += *value;
		taken.fetch_add(1);
	}
	sum.fetch_add(local);
	return nullptr;
}

auto main(void) -> int {
	auto one = ca::mpmc_queue<int>::make(1U);
	if (!one || one->capacity() != 2U)
		return 1;
	if (!one->try_push(1) || !one->try_emplace(2))
		return 2;
	auto full = one->try_push(3);
	if (full || full.error() != ca::errc::no_space || one->size() != 2U)
		return 3;
	for (int lap = 0; lap < 9; ++lap) {
		auto front = one->try_pop();
		if (!front || *front != lap + 1 || !one->try_push(lap + 3))
			return 4;
	}
	if (!one->try_pop() || !one->try_pop() || one->try_pop() || one->size() != 0U)
		return 5;
	if (!one->cleanup() || one->capacity() != 0U)
		return 6;

	auto made = ca::mpmc_queue<long>::make(256U);
	if (!made)
		return 7;
	pthread_t workers[threads * 2];
	work jobs[threads];
	for (int t = 0; t < threads; ++t) {
		jobs[t] = work{&*made, t * per_thread};
		if (pthread_create(&workers[t], nullptr, produce, &jobs[t]) != 0)
			return 8;
		if (pthread_create(&workers[threads + t], nullptr, consume, &*made) != 0)
			return 8;
	}
	for (pthread_t& worker : workers)
		pthread_join(worker, nullptr);
	for (long i = 0; i < total; ++i) {
		if (seen[i] != 1U)
			return 9;
	}
	if (sum.load() != total * (total - 1) / 2 || made->try_pop())
		return 10;
	return made->cleanup() ? 0 : 11;
}
