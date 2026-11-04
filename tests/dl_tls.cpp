// expect: exit 0
// shared: tlsbase dl/tls_base.cpp -Wl,-soname,libtlsbase.so
// shared: tlsuser dl/tls_user.cpp -L{dir} -ltlsbase -Wl,-rpath,$ORIGIN
// shared: tlsie dl/tls_initial_exec.cpp -ftls-model=initial-exec
#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/atomic/spin_pause.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/thread/thread.hpp"
#include "5-collections/format/format.hpp"
#include "5-collections/trace/report.hpp"
#include "6-resources/dl/library.hpp"

static constexpr int THREADS = 8;

using counter_function = int(void);
using address_function = int*(void);

struct functions final {
	counter_function* bump;
	counter_function* hidden;
	counter_function* zeros;
	counter_function* read;
	address_function* address;
};

static functions f{};
static ca::atomic<int> failures{0};
static ca::atomic<int> arrived{0};
static ca::atomic<int> expected{THREADS};
static int* addresses[THREADS];

/* Checks its thread starts from the image, then waits for every other one, so all the blocks
   are alive at once when their addresses are compared. */
static auto check_fresh_thread(void* const argument) noexcept -> void {

	const int index = static_cast<int>(reinterpret_cast<ca::usz>(argument));
	const bool fresh = f.bump() == 6 && f.bump() == 7 && f.hidden() == 2 && f.zeros() == 0 && f.zeros() == 1 && f.read() == 70;
	addresses[index] = f.address();
	if (!fresh || *addresses[index] != 7)
		failures.fetch_add<ca::memory_order::relaxed>(1);
	arrived.fetch_add(1);
	while (arrived.load() < expected.load())
		ca::spin_pause();
}

static auto bind(const ca::dl::library& library) noexcept -> bool {

	const auto bump = library.find_as<counter_function>("tls_bump");
	const auto hidden = library.find_as<counter_function>("tls_hidden");
	const auto zeros = library.find_as<counter_function>("tls_zeros");
	const auto read = library.find_as<counter_function>("tls_user_read");
	const auto address = library.find_as<address_function>("tls_counter_address");
	if (!bump || !hidden || !zeros || !read || !address)
		return false;
	f = functions{*bump, *hidden, *zeros, *read, *address};
	return true;
}

static auto refused(const ca::sv::str path, const ca::sv::str text) noexcept -> bool {

	auto loaded = ca::dl::library::make(path);
	if (loaded)
		return false;
	auto printed = ca::format("{}", loaded.error());
	const bool matches = loaded.error().error() == ca::errc::not_supported && printed && printed->view().contains(text);
	if (printed)
		static_cast<void>(printed->cleanup());
	return loaded.error().cleanup() && matches;
}

auto main(void) -> int {
	auto user = ca::dl::library::make(CA_TEST_LIBRARIES "/libtlsuser.so");
	if (!user || !bind(*user))
		return 1;
	if (f.bump() != 6 || f.bump() != 7 || f.hidden() != 2 || f.hidden() != 3 || f.zeros() != 0 || f.zeros() != 1 || f.read() != 70)
		return 2;
	const auto found = user->find_as<int>("tls_counter");
	if (!found || *found != f.address() || **found != 7)
		return 3;

	ca::sys::thread threads[THREADS];
	int started = 0;
	for (; started < THREADS; ++started) {
		auto made = ca::sys::thread::make(check_fresh_thread, reinterpret_cast<void*>(static_cast<ca::usz>(started)));
		if (!made) {
			expected.store(started);
			if (made.error() != ca::errc::not_supported)
				return 4;
			break;
		}
		threads[started] = static_cast<ca::sys::thread&&>(*made);
	}
	for (int i = 0; i < started; ++i) {
		if (!threads[i].join())
			return 5;
	}
	if (failures.load() != 0)
		return 6;
	for (int i = 0; i < started; ++i) {
		if (addresses[i] == f.address())
			return 7;
		for (int j = 0; j < i; ++j) {
			if (addresses[i] == addresses[j])
				return 8;
		}
	}
	if (f.bump() != 8 || f.read() != 80)
		return 9;

	if (!user->cleanup())
		return 10;
	auto again = ca::dl::library::make(CA_TEST_LIBRARIES "/libtlsuser.so");
	if (!again || !bind(*again) || f.bump() != 6 || f.read() != 60 || !again->cleanup())
		return 11;

	if (!refused(CA_TEST_LIBRARIES "/libtlsie.so", "initial-exec thread-local storage"))
		return 12;
	return 0;
}
