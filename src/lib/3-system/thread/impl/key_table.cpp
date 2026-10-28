#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/sync/mutex.hpp"
#include "3-system/thread/impl/key_table.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

static constexpr ca::usz KEYS = 128U;
static constexpr int PASSES = 4;

static void (*destructors[KEYS])(void*); // file-state: ignore
static constinit ca::atomic<ca::usz> created{0U}; // file-state: ignore
static constinit ca::mutex creating; // file-state: ignore
static constinit thread_local void* values[KEYS]{};

[[nodiscard]] auto ca::sys::impl::create_key(void (* const destructor)(void*)) noexcept -> ca::result<ca::usz> {

	creating.lock();
	const ca::usz key = created.load<ca::memory_order::relaxed>();
	if (key == KEYS) [[unlikely]] {
		creating.unlock();
		return ca::failure{ca::errc::try_again};
	}
	destructors[key] = destructor;
	created.store<ca::memory_order::release>(key + 1U);
	creating.unlock();
	return key;
}

[[nodiscard]] auto ca::sys::impl::set_key(const ca::usz key, void* const value) noexcept -> ca::result<void> {

	if (key >= created.load<ca::memory_order::acquire>()) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	values[key] = value;
	return ca::result<void>{};
}

[[nodiscard]] auto ca::sys::impl::key_value(const ca::usz key) noexcept -> void* {
	return key < created.load<ca::memory_order::acquire>() ? values[key] : nullptr;
}

auto ca::sys::impl::run_key_destructors(void) noexcept -> void {

	const ca::usz count = created.load<ca::memory_order::acquire>();
	for (int pass = 0; pass < PASSES; ++pass) {
		bool ran = false;
		for (ca::usz key = 0U; key < count; ++key) {
			void* const value = values[key];
			if (value == nullptr || destructors[key] == nullptr)
				continue;
			values[key] = nullptr;
			destructors[key](value);
			ran = true;
		}
		if (!ran)
			return;
	}
}

#endif
