#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/usz.hpp"
#include "3-system/memory/free.hpp"
#include "3-system/memory/reallocarray.hpp"
#include "3-system/thread/impl/key_table.hpp"
#include "3-system/thread/impl/thread_exit.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

struct thread_exit_handler final { // ns-leak: ignore
		using self = thread_exit_handler;
		void (*_function)(void*);
		void* _object;
}; // struct thread_exit_handler

static constinit thread_local thread_exit_handler* handlers = nullptr;
static constinit thread_local ca::usz registered = 0U;
static constinit thread_local ca::usz capacity = 0U;

[[nodiscard]] auto ca::sys::impl::at_thread_exit(void (* const function)(void*), void* const object) noexcept -> ca::result<void> {

	if (registered == capacity) {
		const ca::usz grown = capacity == 0U ? 16U : capacity * 2U;
		auto block = ca::sys::reallocarray(handlers, grown, sizeof(thread_exit_handler));
		if (!block) [[unlikely]]
			return block.propagate();
		handlers = static_cast<thread_exit_handler*>(*block);
		capacity = grown;
	}
	handlers[registered++] = thread_exit_handler{function, object};
	return ca::result<void>{};
}

auto ca::sys::impl::run_thread_exit_handlers(void) noexcept -> void {

	while (registered != 0U) {
		const thread_exit_handler handler = handlers[--registered];
		handler._function(handler._object);
	}
	ca::sys::free(handlers);
	handlers = nullptr;
	capacity = 0U;
}

auto ca::sys::impl::finish_thread(void) noexcept -> void {

	ca::sys::impl::run_thread_exit_handlers();
	ca::sys::impl::run_key_destructors();
}

#endif
