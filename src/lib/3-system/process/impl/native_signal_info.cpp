#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"
#include "3-system/process/impl/native_signal_info.hpp"

#include <signal.h>


static_assert(ca::sys::signal_codes::user == SI_USER);
static_assert(ca::sys::signal_codes::queue == SI_QUEUE);

[[nodiscard]] auto ca::sys::impl::from_native_info(const void* const native) noexcept -> ca::sys::signal_info {

	const siginfo_t* const in = static_cast<const siginfo_t*>(native);
	const ca::i64 value = static_cast<ca::i64>(reinterpret_cast<__INTPTR_TYPE__>(in->si_value.sival_ptr));
	return ca::sys::signal_info{in->si_signo, in->si_code, static_cast<ca::i32>(in->si_pid), static_cast<ca::u32>(in->si_uid), value};
}
