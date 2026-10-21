// expect: exit 0
// link: libc
#include "3-system/sync/condition.hpp"
#include "3-system/sync/latch.hpp"
#include "3-system/sync/mutex.hpp"
#include "3-system/sync/once.hpp"
#include "3-system/sync/semaphore.hpp"

#include <pthread.h>

static constexpr int threads = 8;
static constexpr long rounds = 100000;

static constinit ca::mutex lock;
static long counter = 0;

static constinit ca::condition changed;
static int queue[16];
static int queued = 0;
static long consumed = 0;

static constinit ca::once setup;
static ca::atomic<int> setups{0};

static ca::latch arrived{threads};
static ca::semaphore slots{2U};
static ca::atomic<int> inside{0};
static ca::atomic<int> peak{0};

static auto count(void*) -> void* {

	for (long i = 0; i < rounds; ++i) {
		lock.lock();
		++counter;
		lock.unlock();
	}
	return nullptr;
}

static auto produce(void*) -> void* {

	for (int i = 1; i <= 1000; ++i) {
		lock.lock();
		changed.wait(lock, [] { return queued < 16; });
		queue[queued++] = i;
		lock.unlock();
		changed.notify_all();
	}
	return nullptr;
}

static auto consume(void*) -> void* {

	for (int i = 0; i < 1000; ++i) {
		lock.lock();
		changed.wait(lock, [] { return queued > 0; });
		consumed += queue[--queued];
		lock.unlock();
		changed.notify_all();
	}
	return nullptr;
}

static auto start_once(void*) -> void* {

	static_cast<void>(setup.call([] {
		for (int i = 0; i < 1000; ++i)
			setups.fetch_add(0);
		setups.fetch_add(1);
	}));
	arrived.arrive_and_wait();
	for (int i = 0; i < 1000; ++i) {
		slots.acquire();
		const int now = inside.fetch_add(1) + 1;
		int seen = peak.load();
		while (seen < now && !peak.compare_exchange_weak(seen, now)) {
		}
		inside.fetch_sub(1);
		slots.release();
	}
	return nullptr;
}

static auto run(void* (*body)(void*), const int count) -> bool {

	pthread_t ids[threads];
	for (int i = 0; i < count; ++i) {
		if (pthread_create(&ids[i], nullptr, body, nullptr) != 0)
			return false;
	}
	for (int i = 0; i < count; ++i)
		pthread_join(ids[i], nullptr);
	return true;
}

auto main(void) -> int {
	if (!run(&count, threads) || counter != threads * rounds)
		return 1;
	pthread_t producers[2];
	pthread_t consumers[2];
	for (int i = 0; i < 2; ++i) {
		if (pthread_create(&producers[i], nullptr, &produce, nullptr) != 0 || pthread_create(&consumers[i], nullptr, &consume, nullptr) != 0)
			return 2;
	}
	for (int i = 0; i < 2; ++i) {
		pthread_join(producers[i], nullptr);
		pthread_join(consumers[i], nullptr);
	}
	if (consumed != 2L * 500500L || queued != 0)
		return 3;
	if (!run(&start_once, threads) || setups.load() != 1 || !arrived.try_wait())
		return 4;
	return peak.load() >= 1 && peak.load() <= 2 ? 0 : 5;
}
