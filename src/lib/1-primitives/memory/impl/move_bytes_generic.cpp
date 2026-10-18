#include "1-primitives/memory/impl/move_bytes.hpp"

#include "1-primitives/config/architecture.hpp"

#if !ca_architecture(x86_64)

static auto move_builtin(void* const to, const void* const from, const ca::usz count) noexcept -> void* {

	if (count != 0U)
		__builtin_memmove(to, from, count);
	return to;
}

constinit ca::atomic<ca::impl::move_bytes_function> ca::impl::move_bytes_variant{&move_builtin}; // ns-leak: ignore

#endif
