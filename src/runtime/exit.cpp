/* exit, atexit, __cxa_atexit, __cxa_thread_atexit and __dso_handle for programs linked without
   libc (see start_x86_64.cpp). Static destructors register through __cxa_atexit, those of
   thread_local objects through __cxa_thread_atexit (the main thread's: ca starts no thread
   yet); exit runs the thread_local ones in reverse, then the others in reverse (as glibc, those
   they register too), then fini_array in reverse, then exit_group. */

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
static ca::runtime::impl::exit_handler thread_handlers[HANDLERS]; // file-state: ignore
static ca::usz thread_registered = 0U; // file-state: ignore
static constinit ca::atomic<int> registering{0}; // file-state: ignore

static auto lock(void) noexcept -> void {
	while (registering.exchange<ca::memory_order::acquire>(1) != 0) {
	}
}

static auto unlock(void) noexcept -> void {
	registering.store<ca::memory_order::release>(0);
}

static auto add(ca::runtime::impl::exit_handler* const list, ca::usz& count, const ca::runtime::impl::exit_handler handler) noexcept -> int {

	lock();
	const bool room = count < HANDLERS;
	if (room)
		list[count++] = handler;
	unlock();
	return room ? 0 : -1;
}

/* Runs the handlers of list in reverse, those they register too. */
static auto run(ca::runtime::impl::exit_handler* const list, ca::usz& count) noexcept -> void {
	for (;;) {
		lock();
		if (count == 0U) {
			unlock();
			return;
		}
		const ca::runtime::impl::exit_handler handler = list[--count];
		unlock();
		if (handler._plain != nullptr)
			handler._plain();
		else
			handler._function(handler._argument);
	}
}

extern "C" {

	[[gnu::visibility("hidden")]] void* __dso_handle = &__dso_handle; // ns-leak: ignore

	auto __cxa_atexit(void (*const function)(void*), void* const argument, void*) noexcept -> int { // ns-leak: ignore
		return add(handlers, registered, {function, nullptr, argument});
	}

	auto atexit(void (*const function)(void)) noexcept -> int { // ns-leak: ignore
		return add(handlers, registered, {nullptr, function, nullptr});
	}

	auto __cxa_thread_atexit_impl(void (*const function)(void*), void* const object, void*) noexcept -> int { // ns-leak: ignore
		return add(thread_handlers, thread_registered, {function, nullptr, object});
	}

	auto __cxa_thread_atexit(void (*const function)(void*), void* const object, void* const dso) noexcept -> int { // ns-leak: ignore
		return __cxa_thread_atexit_impl(function, object, dso);
	}

	[[noreturn]] auto exit(const int status) noexcept -> void { // ns-leak: ignore

		run(thread_handlers, thread_registered);
		run(handlers, registered);
		for (fini_function* f = __fini_array_end; f != __fini_array_start;)
			(*--f)();
		ca::syscall::exit_group(status);
	}

} // extern "C"

#endif
