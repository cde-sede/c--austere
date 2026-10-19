#include "1-primitives/memory/impl/compare_bytes.hpp"

#include "1-primitives/config/architecture.hpp"

#if !ca_architecture(x86_64)

static auto compare_builtin(const void* const a, const void* const b, const ca::usz count) noexcept -> int {
	return count != 0U ? __builtin_memcmp(a, b, count) : 0;
}

constinit ca::atomic<ca::impl::compare_bytes_function> ca::impl::compare_bytes_variant{&compare_builtin}; // ns-leak: ignore

constinit ca::atomic<ca::impl::compare_bytes_function> ca::impl::differ_bytes_variant{&compare_builtin}; // ns-leak: ignore

#endif
