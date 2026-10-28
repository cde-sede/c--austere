// expect: exit 0
// link: libc
#include "3-system/error/errc.hpp"
#include "5-collections/concurrent/spsc_queue.hpp"

#include <pthread.h>

static constexpr long count = 1000000;

struct only_moved final {
	long _value;
	explicit only_moved(const long value) noexcept
	: _value{value} {
	}
	only_moved(const only_moved&) = delete;
	only_moved(only_moved&& other) noexcept
	: _value{other._value} {
		other._value = -1;
	}
	auto operator=(const only_moved&) -> only_moved& = delete;
};

static int cleaned = 0;

struct tracked final {
	int _value;
	[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {
		++cleaned;
		if (_value == 2)
			return ca::failure{ca::errc::io_error};
		return ca::result<void>{};
	}
};

static auto produce(void* const argument) -> void* {

	auto& queue = *static_cast<ca::spsc_queue<only_moved>*>(argument);
	for (long i = 1; i <= count; ++i) {
		only_moved value{i};
		while (!queue.try_push(ca::move(value))) {
			if (value._value != i)
				__builtin_trap();
		}
	}
	return nullptr;
}

auto main(void) -> int {
	auto made = ca::spsc_queue<int>::make(3U);
	if (!made || made->capacity() != 4U || made->size() != 0U || made->try_pop())
		return 1;
	auto queue = ca::move(*made);
	for (int i = 0; i < 4; ++i) {
		if (!queue.try_push(i))
			return 2;
	}
	auto full = queue.try_push(4);
	if (full || full.error() != ca::errc::no_space || queue.size() != 4U)
		return 3;
	for (int lap = 0; lap < 10; ++lap) {
		auto front = queue.try_pop();
		if (!front || *front != lap || !queue.try_emplace(lap + 4))
			return 4;
	}
	if (!queue.cleanup() || queue.capacity() != 0U || !queue.cleanup())
		return 5;
	auto empty = ca::spsc_queue<int>::make(0U);
	if (empty || empty.error() != ca::errc::invalid_argument)
		return 6;

	auto items = ca::spsc_queue<tracked>::make(8U);
	if (!items || !items->try_emplace(1) || !items->try_emplace(2) || !items->try_emplace(3))
		return 7;
	auto popped = items->try_pop();
	if (!popped || popped->_value != 1 || !popped->cleanup())
		return 8;
	auto left = items->cleanup();
	if (left || left.error() != ca::errc::io_error || cleaned != 3)
		return 9;

	auto shared = ca::spsc_queue<only_moved>::make(64U);
	if (!shared)
		return 10;
	pthread_t producer;
	if (pthread_create(&producer, nullptr, produce, &*shared) != 0)
		return 11;
	long expected = 1;
	while (expected <= count) {
		auto value = shared->try_pop();
		if (!value)
			continue;
		if (value->_value != expected)
			return 12;
		++expected;
	}
	pthread_join(producer, nullptr);
	if (shared->try_pop())
		return 13;
	return shared->cleanup() ? 0 : 14;
}
