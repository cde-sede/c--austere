#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/thread/impl/key_table.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

static constexpr ca::usz KEYS = 128U;

static void* values[KEYS]; // file-state: ignore
static ca::usz created = 0U; // file-state: ignore

[[nodiscard]] auto ca::sys::impl::create_key(void) noexcept -> ca::result<ca::usz> {

	if (created == KEYS) [[unlikely]]
		return ca::failure{ca::errc::try_again};
	return created++;
}

[[nodiscard]] auto ca::sys::impl::set_key(const ca::usz key, void* const value) noexcept -> ca::result<void> {

	if (key >= created) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	values[key] = value;
	return ca::result<void>{};
}

[[nodiscard]] auto ca::sys::impl::key_value(const ca::usz key) noexcept -> void* {
	return key < created ? values[key] : nullptr;
}

#endif
