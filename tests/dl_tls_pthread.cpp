// expect: exit 0
// link: libc
// shared: tlsbase dl/tls_base.cpp -Wl,-soname,libtlsbase.so
// The loader's thread-local storage in threads libc starts: each pthread gets its own copy,
// alongside libc's own TLS, which the loader never touches.
#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/spin_pause.hpp"
#include "6-resources/dl/library.hpp"

#include <pthread.h>

static constexpr int THREADS = 4;

static int (*bump)(void) = nullptr;
static int* (*address)(void) = nullptr;
static ca::atomic<int> failures{0};
static ca::atomic<int> arrived{0};
static int* addresses[THREADS];

static auto run(void* const argument) -> void* {

	const int index = static_cast<int>(reinterpret_cast<ca::usz>(argument));
	if (bump() != 6 || bump() != 7)
		failures.fetch_add(1);
	addresses[index] = address();
	arrived.fetch_add(1);
	while (arrived.load() < THREADS)
		ca::spin_pause();
	return nullptr;
}

auto main(void) -> int {
	auto base = ca::dl::library::make(CA_TEST_LIBRARIES "/libtlsbase.so");
	if (!base)
		return 1;
	const auto b = base->find_as<int(void)>("tls_bump");
	const auto a = base->find_as<int*(void)>("tls_counter_address");
	if (!b || !a)
		return 2;
	   bump = *b;
	address = *a;
	if (bump() != 6)
		return 3;
	pthread_t threads[THREADS];
	for (int i = 0; i < THREADS; ++i) {
		if (pthread_create(&threads[i], nullptr, run, reinterpret_cast<void*>(static_cast<ca::usz>(i))) != 0)
			return 4;
	}
	for (int i = 0; i < THREADS; ++i) {
		if (pthread_join(threads[i], nullptr) != 0)
			return 5;
	}
	if (failures.load() != 0 || bump() != 7)
		return 6;
	for (int i = 0; i < THREADS; ++i) {
		if (addresses[i] == address())
			return 7;
		for (int j = 0; j < i; ++j) {
			if (addresses[i] == addresses[j])
				return 8;
		}
	}
	return base->cleanup() ? 0 : 9;
}
