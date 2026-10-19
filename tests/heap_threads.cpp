// expect: exit 0
// link: libc
#include "1-primitives/atomic/atomic.hpp"
#include "4-lifecycle/memory/heap.hpp"

#include <pthread.h>

static constexpr ca::usz count = 200000U;
static constexpr ca::usz rounds = 4U;
static void* handoff[rounds][count];
static ca::atomic<ca::usz> produced[rounds];

static auto produce(void* const raw) -> void* {

	const ca::usz round = reinterpret_cast<ca::usz>(raw);
	for (ca::usz i = 0U; i < count; ++i) {
		auto block = ca::heap{}.allocate(16U + (i % 7U) * 40U, 8U);
		if (!block)
			return raw;
		*static_cast<ca::usz*>(block->data()) = i;
		handoff[round][i] = block->data();
		produced[round].store(i + 1U);
	}
	return nullptr;
}

static auto consume(void* const raw) -> void* {

	const ca::usz round = reinterpret_cast<ca::usz>(raw);
	for (ca::usz i = 0U; i < count; ++i) {
		while (produced[round].load() <= i) {
		}
		void* const block = handoff[round][i];
		if (*static_cast<ca::usz*>(block) != i)
			return raw;
		ca::heap{}.deallocate(ca::allocation{block, 16U + (i % 7U) * 40U}, 8U);
	}
	return nullptr;
}

auto main(void) -> int {
	for (ca::usz round = 0U; round < rounds; ++round) {
		pthread_t producer{};
		pthread_t consumer{};
		void* const index = reinterpret_cast<void*>(round);
		if (pthread_create(&producer, nullptr, &produce, index) != 0 || pthread_create(&consumer, nullptr, &consume, index) != 0)
			return 1;
		void* produced_result = nullptr;
		void* consumed_result = nullptr;
		pthread_join(producer, &produced_result);
		pthread_join(consumer, &consumed_result);
		if (produced_result != nullptr || consumed_result != nullptr)
			return 2;
	}
	auto block = ca::heap{}.allocate(64U, 8U);
	if (!block)
		return 3;
	ca::heap{}.deallocate(*block, 8U);
	return 0;
}
