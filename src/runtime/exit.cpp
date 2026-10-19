/* exit, atexit, __cxa_atexit and __dso_handle for programs linked without libc (see
   start_x86_64.cpp). Static destructors register through __cxa_atexit; exit runs the handlers
   in reverse (those they register too), then fini_array in reverse, then exit_group. */

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/usz.hpp"
#include "3-system/syscall/syscalls.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

using fini_function = void (*)(void);

extern "C" [[gnu::visibility("hidden")]] fini_function __fini_array_start[];
extern "C" [[gnu::visibility("hidden")]] fini_function __fini_array_end[];

namespace ca {


	namespace runtime {


		namespace impl {

			/* @DOC One handler of exit: function(argument), or plain() for atexit. */
			struct exit_handler final {
					using self = ca::runtime::impl::exit_handler;
					void (*_function)(void*);
					void (*_plain)(void);
					void* _argument;
			}; // struct exit_handler

		} // namespace impl

	} // namespace runtime

} // namespace ca

static constexpr ca::usz HANDLERS = 1024U;

static ca::runtime::impl::exit_handler handlers[HANDLERS]; // file-state: ignore
static ca::usz registered = 0U; // file-state: ignore
static constinit ca::atomic<int> registering{0}; // file-state: ignore

static auto lock(void) noexcept -> void {
	while (registering.exchange<ca::memory_order::acquire>(1) != 0) {
	}
}

static auto unlock(void) noexcept -> void {
	registering.store<ca::memory_order::release>(0);
}

static auto add(const ca::runtime::impl::exit_handler handler) noexcept -> int {

	lock();
	const bool room = registered < HANDLERS;
	if (room)
		handlers[registered++] = handler;
	unlock();
	return room ? 0 : -1;
}

extern "C" {

	[[gnu::visibility("hidden")]] void* __dso_handle = &__dso_handle; // ns-leak: ignore

	auto __cxa_atexit(void (*const function)(void*), void* const argument, void*) noexcept -> int { // ns-leak: ignore
		return add({function, nullptr, argument});
	}

	auto atexit(void (*const function)(void)) noexcept -> int { // ns-leak: ignore
		return add({nullptr, function, nullptr});
	}

	[[noreturn]] auto exit(const int status) noexcept -> void { // ns-leak: ignore

		for (;;) {
			lock();
			if (registered == 0U) {
				unlock();
				break;
			}
			const ca::runtime::impl::exit_handler handler = handlers[--registered];
			unlock();
			if (handler._plain != nullptr)
				handler._plain();
			else
				handler._function(handler._argument);
		}
		for (fini_function* f = __fini_array_end; f != __fini_array_start;)
			(*--f)();
		ca::syscall::exit_group(status);
	}

} // extern "C"

#endif
