#include "1-primitives/memory/impl/find_bytes.hpp"

#include "1-primitives/config/architecture.hpp"

#if !ca_architecture(x86_64)

static auto find_builtin(const void* const data, const int value, const ca::usz count) noexcept -> void* {
	return count != 0U ? __builtin_memchr(data, value, count) : nullptr;
}

static auto length_builtin(const char* const text) noexcept -> ca::usz {
	return __builtin_strlen(text);
}

constinit ca::atomic<ca::impl::find_byte_function> ca::impl::find_byte_variant{&find_builtin}; // ns-leak: ignore

constinit ca::atomic<ca::impl::string_length_function> ca::impl::string_length_variant{&length_builtin}; // ns-leak: ignore

#endif
