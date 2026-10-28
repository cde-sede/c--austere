// Benchmarks for the concurrent queues: ca::spsc_queue, ca::mpmc_queue, ca::channel and
// ca::io::spsc_ring_buffer. -DUSE_STD runs the same cases on what std offers for a queue
// shared between threads: a std::deque behind a std::mutex, plus two std::condition_variable
// for the blocking channel (notified after every operation, as a bounded buffer must). One
// thread pushes and pops in turn (the harness pins one CPU), so the figures are the cost of
// the operations, without contention or cache-line transfers between cores.

#include "measure.hpp"

static const int ROUNDS = 1000000;
static const int BATCH = 64;
static const unsigned CAPACITY = 4096U;

static char message[64];

#ifdef USE_STD
#include <algorithm>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

template <typename T>
struct locked_queue final {
	std::mutex lock;
	std::deque<T> items;
	auto push(const T value) -> bool {
		const std::lock_guard<std::mutex> held{lock};
		if (items.size() == CAPACITY)
			return false;
		items.push_back(value);
		return true;
	}
	auto pop(void) -> std::optional<T> {
		const std::lock_guard<std::mutex> held{lock};
		if (items.empty())
			return std::nullopt;
		T value = items.front();
		items.pop_front();
		return value;
	}
};

struct bounded_buffer final {
	std::mutex lock;
	std::condition_variable not_empty;
	std::condition_variable not_full;
	std::deque<int> items;
	auto push(const int value) -> void {
		{
			std::unique_lock<std::mutex> held{lock};
			not_full.wait(held, [this] { return items.size() < CAPACITY; });
			items.push_back(value);
		}
		not_empty.notify_one();
	}
	auto pop(void) -> int {
		int value;
		{
			std::unique_lock<std::mutex> held{lock};
			not_empty.wait(held, [this] { return !items.empty(); });
			value = items.front();
			items.pop_front();
		}
		not_full.notify_one();
		return value;
	}
};

template <typename Q>
NI static auto alternating(void) -> void {
	Q queue;
	uint64_t sum = 0;
	for (int i = 0; i < ROUNDS; ++i) {
		static_cast<void>(queue.push(i));
		sum += static_cast<uint64_t>(*queue.pop());
	}
	sink = sum;
}

template <typename Q>
NI static auto batched(void) -> void {
	Q queue;
	uint64_t sum = 0;
	for (int i = 0; i < ROUNDS / BATCH; ++i) {
		for (int j = 0; j < BATCH; ++j)
			static_cast<void>(queue.push(i + j));
		for (int j = 0; j < BATCH; ++j)
			sum += static_cast<uint64_t>(*queue.pop());
	}
	sink = sum;
}

NI static auto channel_alternating(void) -> void {
	bounded_buffer channel;
	uint64_t sum = 0;
	for (int i = 0; i < ROUNDS; ++i) {
		channel.push(i);
		sum += static_cast<uint64_t>(channel.pop());
	}
	sink = sum;
}

NI static auto channel_batched(void) -> void {
	bounded_buffer channel;
	uint64_t sum = 0;
	for (int i = 0; i < ROUNDS / BATCH; ++i) {
		for (int j = 0; j < BATCH; ++j)
			channel.push(i + j);
		for (int j = 0; j < BATCH; ++j)
			sum += static_cast<uint64_t>(channel.pop());
	}
	sink = sum;
}

NI static auto bytes_fixed(void) -> void {
	std::mutex lock;
	std::deque<char> fifo;
	char out[64];
	uint64_t sum = 0;
	for (int i = 0; i < ROUNDS; ++i) {
		{
			const std::lock_guard<std::mutex> held{lock};
			fifo.insert(fifo.end(), message, message + 64);
		}
		{
			const std::lock_guard<std::mutex> held{lock};
			std::copy_n(fifo.begin(), 64, out);
			fifo.erase(fifo.begin(), fifo.begin() + 64);
		}
		sum += static_cast<unsigned char>(out[i & 63]);
	}
	sink = sum;
}

NI static auto bytes_batched(void) -> void {
	std::mutex lock;
	std::deque<char> fifo;
	char out[64];
	uint64_t sum = 0;
	for (int i = 0; i < ROUNDS / BATCH; ++i) {
		for (int j = 0; j < BATCH; ++j) {
			const std::lock_guard<std::mutex> held{lock};
			fifo.insert(fifo.end(), message, message + 32);
		}
		for (int j = 0; j < BATCH; ++j) {
			const std::lock_guard<std::mutex> held{lock};
			std::copy_n(fifo.begin(), 32, out);
			fifo.erase(fifo.begin(), fifo.begin() + 32);
			sum += static_cast<unsigned char>(out[j & 31]);
		}
	}
	sink = sum;
}

#define SPSC alternating<locked_queue<int>>
#define SPSC_BATCHED batched<locked_queue<int>>
#define MPMC alternating<locked_queue<int>>
#define MPMC_BATCHED batched<locked_queue<int>>
#else
#include "5-collections/concurrent/channel.hpp"
#include "5-collections/concurrent/mpmc_queue.hpp"
#include "5-collections/concurrent/spsc_queue.hpp"
#include "5-collections/io/spsc_ring_buffer.hpp"

template <typename Q>
NI static auto alternating(void) -> void {
	auto queue = Q::make(CAPACITY);
	uint64_t sum = 0;
	for (int i = 0; i < ROUNDS; ++i) {
		static_cast<void>(queue->try_push(i));
		sum += static_cast<uint64_t>(*queue->try_pop());
	}
	static_cast<void>(queue->cleanup());
	sink = sum;
}

template <typename Q>
NI static auto batched(void) -> void {
	auto queue = Q::make(CAPACITY);
	uint64_t sum = 0;
	for (int i = 0; i < ROUNDS / BATCH; ++i) {
		for (int j = 0; j < BATCH; ++j)
			static_cast<void>(queue->try_push(i + j));
		for (int j = 0; j < BATCH; ++j)
			sum += static_cast<uint64_t>(*queue->try_pop());
	}
	static_cast<void>(queue->cleanup());
	sink = sum;
}

NI static auto channel_alternating(void) -> void {
	auto channel = ca::channel<int>::make(CAPACITY);
	uint64_t sum = 0;
	for (int i = 0; i < ROUNDS; ++i) {
		static_cast<void>(channel->push(i));
		sum += static_cast<uint64_t>(*channel->pop());
	}
	static_cast<void>(channel->cleanup());
	sink = sum;
}

NI static auto channel_batched(void) -> void {
	auto channel = ca::channel<int>::make(CAPACITY);
	uint64_t sum = 0;
	for (int i = 0; i < ROUNDS / BATCH; ++i) {
		for (int j = 0; j < BATCH; ++j)
			static_cast<void>(channel->push(i + j));
		for (int j = 0; j < BATCH; ++j)
			sum += static_cast<uint64_t>(*channel->pop());
	}
	static_cast<void>(channel->cleanup());
	sink = sum;
}

NI static auto bytes_fixed(void) -> void {
	auto ring = ca::io::spsc_ring_buffer<>::make(CAPACITY);
	char out[64];
	uint64_t sum = 0;
	for (int i = 0; i < ROUNDS; ++i) {
		static_cast<void>(ring->write(ca::span{message, 64U}));
		static_cast<void>(ring->read(out));
		sum += static_cast<unsigned char>(out[i & 63]);
	}
	static_cast<void>(ring->cleanup());
	sink = sum;
}

NI static auto bytes_batched(void) -> void {
	auto ring = ca::io::spsc_ring_buffer<>::make(CAPACITY);
	char out[64];
	uint64_t sum = 0;
	for (int i = 0; i < ROUNDS / BATCH; ++i) {
		for (int j = 0; j < BATCH; ++j)
			static_cast<void>(ring->write(ca::span{message, 32U}));
		for (int j = 0; j < BATCH; ++j) {
			static_cast<void>(ring->read(ca::span{out, 32U}));
			sum += static_cast<unsigned char>(out[j & 31]);
		}
	}
	static_cast<void>(ring->cleanup());
	sink = sum;
}

#define SPSC alternating<ca::spsc_queue<int>>
#define SPSC_BATCHED batched<ca::spsc_queue<int>>
#define MPMC alternating<ca::mpmc_queue<int>>
#define MPMC_BATCHED batched<ca::mpmc_queue<int>>
#endif


// -- MAIN -------------------------------------------------------------------

auto main(void) -> int {
	for (unsigned i = 0; i < sizeof message; ++i)
		message[i] = static_cast<char>('a' + i % 26U);
	if (!open_counters())
		return 1;
	section("spsc");
	bench("push + pop, 1e6 ints", SPSC);
	bench("64 pushes then 64 pops, 1e6 ints", "ca: each side reads the other's index once per batch", SPSC_BATCHED);
	section("mpmc");
	bench("push + pop, 1e6 ints", MPMC);
	bench("64 pushes then 64 pops, 1e6 ints", "ca: a compare-exchange per operation", MPMC_BATCHED);
	section("channel");
	bench("push + pop, 1e6 ints", "std: notify_one after each, nobody waiting", channel_alternating);
	bench("64 pushes then 64 pops, 1e6 ints", channel_batched);
	section("bytes");
	bench("64 B write + read, 1e6", bytes_fixed);
	bench("64 x 32 B writes then 64 reads, 1e6", bytes_batched);
	return 0;
}
