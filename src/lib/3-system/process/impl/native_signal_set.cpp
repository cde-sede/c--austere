#include "3-system/process/impl/native_signal_set.hpp"

#include <signal.h>


auto ca::sys::impl::to_native(const ca::sys::signal_set& set, void* const native) noexcept -> void {

	sigset_t* const out = static_cast<sigset_t*>(native);
	::sigemptyset(out);
	for (int signal = 1; signal <= 64; ++signal) {
		if (set.contains(signal))
			::sigaddset(out, signal);
	}
}

[[nodiscard]] auto ca::sys::impl::from_native(const void* const native) noexcept -> ca::sys::signal_set {

	const sigset_t* const in = static_cast<const sigset_t*>(native);
	ca::sys::signal_set set;
	for (int signal = 1; signal <= 64; ++signal) {
		if (::sigismember(in, signal) == 1)
			set.add(signal);
	}
	return set;
}
