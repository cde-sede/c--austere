#include "1-primitives/memory/impl/fill_bytes.hpp"

#include "1-primitives/config/architecture.hpp"

#if !ca_architecture(x86_64)

static auto fill_builtin(void* const to, const int value, const ca::usz count) noexcept -> void* {

	if (count != 0U)
		__builtin_memset(to, value, count);
	return to;
}

constinit ca::atomic<ca::impl::fill_bytes_function> ca::impl::fill_bytes_variant{&fill_builtin}; // ns-leak: ignore

#endif
