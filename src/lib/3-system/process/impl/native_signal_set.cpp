#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u64.hpp"
#include "3-system/process/impl/native_signal_set.hpp"

#include <signal.h>


#if ca_platform(linux) && ca_architecture(x86_64)

/* sigset_t holds the kernel's 64-bit set in its first word, signal n at bit n - 1, then words
   only libc uses. */
auto ca::sys::impl::to_native(const ca::sys::signal_set& set, void* const native) noexcept -> void {

	ca::u64 bits = 0U;
	for (int signal = 1; signal <= 64; ++signal) {
		if (set.contains(signal))
			bits |= ca::u64{1U} << (signal - 1);
	}
	*static_cast<sigset_t*>(native) = sigset_t{};
	__builtin_memcpy(native, &bits, sizeof(bits));
}

[[nodiscard]] auto ca::sys::impl::from_native(const void* const native) noexcept -> ca::sys::signal_set {

	ca::u64 bits;
	__builtin_memcpy(&bits, native, sizeof(bits));
	ca::sys::signal_set set;
	for (int signal = 1; signal <= 64; ++signal) {
		if ((bits >> (signal - 1) & 1U) != 0U)
			set.add(signal);
	}
	return set;
}

#else

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

#endif
