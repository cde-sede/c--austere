// expect: exit 0
// link: both
#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/integers/u64.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/memory/free.hpp"
#include "3-system/memory/mallocarray.hpp"
#include "3-system/syscall/syscalls.hpp"
#include "3-system/thread/impl/key_table.hpp"
#include "3-system/thread/impl/thread_exit.hpp"
#include "3-system/thread/impl/tls.hpp"
#include "3-system/thread/thread.hpp"
#include "3-system/thread/thread_key_create.hpp"
#include "3-system/thread/thread_key_set.hpp"

static constexpr int THREADS = 32;

alignas(128) static constinit thread_local ca::u64 initialized = 0x1234567890abcdefU;
static constinit thread_local int zeroed = 0;

static constinit ca::atomic<int> failures{0};
static constinit ca::atomic<int> exit_handlers{0};
static constinit ca::atomic<int> key_exits{0};
static constinit ca::atomic<int> stage_exits{0};
static constinit ca::atomic<bool> finished{false};
static ca::usz key = 0U;
static ca::sys::thread_key stage_key{0U};
static int cells[THREADS];

static auto fail(void) noexcept -> void {
	failures.fetch_add<ca::memory_order::relaxed>(1);
}

static auto count_handler(void*) noexcept -> void {
	exit_handlers.fetch_add<ca::memory_order::relaxed>(1);
}

static auto count_key(void*) noexcept -> void {
	key_exits.fetch_add<ca::memory_order::relaxed>(1);
}

static auto count_stage(void*) noexcept -> void {
	stage_exits.fetch_add<ca::memory_order::relaxed>(1);
}

static auto worker(void* const argument) noexcept -> void {

	int* const cell = static_cast<int*>(argument);
	if (initialized != 0x1234567890abcdefU || zeroed != 0)
		fail();
	initialized = reinterpret_cast<ca::u64>(cell);
	zeroed = 5;
	if (!ca::sys::impl::at_thread_exit(&count_handler, nullptr) || !ca::sys::impl::set_key(key, cell) || !ca::sys::thread_key_set(stage_key, cell))
		fail();
	for (int i = 0; i < 100; ++i) {
		auto block = ca::sys::mallocarray(64U + static_cast<ca::usz>(i), 8U);
		if (!block) {
			fail();
			continue;
		}
		*static_cast<int*>(*block) = i;
		ca::sys::free(*block);
	}
	if (initialized != reinterpret_cast<ca::u64>(cell) || zeroed != 5)
		fail();
	*cell = 1;
}

/* Long enough for main to be asleep in join, which only the kernel's wake then ends. */
static auto slow(void*) noexcept -> void {

	const ca::u64 time[2] = {0U, 50000000U};
	static_cast<void>(ca::syscall::nanosleep(time, nullptr));
	finished.store<ca::memory_order::release>(true);
}

auto main(void) -> int {
	if (ca::sys::impl::program_tls._align == 0U) {
		auto refused = ca::sys::thread::make(&slow, nullptr);
		return !refused && refused.error() == ca::errc::not_supported ? 0 : 1;
	}
	auto first = ca::sys::impl::create_key(&count_key);
	auto stage = ca::sys::thread_key_create(&count_stage);
	if (!first || !stage)
		return 2;
	key = *first;
	stage_key = *stage;

	if (ca::sys::thread::make(&slow, nullptr, ca::sys::thread::MINIMUM_STACK_SIZE - 1U))
		return 3;
	if (ca::sys::thread::make(&slow, nullptr, ~ca::usz{0}))
		return 4;

	ca::sys::thread threads[THREADS];
	for (int i = 0; i < THREADS; ++i) {
		auto made = ca::sys::thread::make(&worker, &cells[i], ca::sys::thread::MINIMUM_STACK_SIZE);
		if (!made)
			return 5;
		threads[i] = static_cast<ca::sys::thread&&>(*made);
	}
	for (ca::sys::thread& t : threads) {
		if (!t.is_live() || !t.join() || t.is_live())
			return 6;
	}
	for (const int cell : cells) {
		if (cell != 1)
			return 7;
	}
	if (failures.load() != 0)
		return 8;
	if (exit_handlers.load() != THREADS || key_exits.load() != THREADS || stage_exits.load() != THREADS)
		return 9;
	if (initialized != 0x1234567890abcdefU || zeroed != 0)
		return 10;

	auto waited = ca::sys::thread::make(&slow, nullptr);
	if (!waited || waited->id() <= 0)
		return 11;
	if (!waited->cleanup() || !finished.load<ca::memory_order::acquire>())
		return 12;
	if (!waited->cleanup() || waited->join())
		return 13;
	return 0;
}
