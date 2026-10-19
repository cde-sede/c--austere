#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/syscall/syscalls.hpp"
#include "3-system/syscall/vdso.hpp"
#include "3-system/time/now.hpp"

#include <time.h>


#if ca_platform(linux) && ca_architecture(x86_64)

namespace ca {


	namespace sys {


		namespace impl {

			/* @DOC clock_gettime as the vDSO exports it: 0, or -errno. */
			using clock_function = auto (*)(clockid_t, timespec*) noexcept -> int;

		} // namespace impl

	} // namespace sys

} // namespace ca

static auto by_syscall(const clockid_t id, timespec* const time) noexcept -> int {
	return ca::syscall::clock_gettime(id, time) ? 0 : -1;
}

static auto resolve(clockid_t id, timespec* time) noexcept -> int;

/* __vdso_clock_gettime once found (no kernel entry: ~20 ns), the syscall without a vDSO. */
static constinit ca::atomic<ca::sys::impl::clock_function> clock_variant{&resolve}; // file-state: ignore

static auto resolve(const clockid_t id, timespec* const time) noexcept -> int {

	void* const symbol = ca::syscall::vdso_symbol("__vdso_clock_gettime");
	const ca::sys::impl::clock_function best = symbol != nullptr ? __builtin_bit_cast(ca::sys::impl::clock_function, symbol) : &by_syscall;
	clock_variant.store<ca::memory_order::relaxed>(best);
	return best(id, time);
}

#endif

[[nodiscard]] auto ca::sys::now(const ca::sys::clock source) noexcept -> ca::i64 {

	const clockid_t id = source == ca::sys::clock::realtime ? CLOCK_REALTIME
		: source == ca::sys::clock::monotonic ? CLOCK_MONOTONIC
		: source == ca::sys::clock::process_cpu ? CLOCK_PROCESS_CPUTIME_ID : CLOCK_THREAD_CPUTIME_ID;
	timespec time{};
#if ca_platform(linux) && ca_architecture(x86_64)
	static_cast<void>(clock_variant.load<ca::memory_order::relaxed>()(id, &time));
#else
	::clock_gettime(id, &time);
#endif
	return static_cast<ca::i64>(time.tv_sec) * 1000000000 + static_cast<ca::i64>(time.tv_nsec);
}
