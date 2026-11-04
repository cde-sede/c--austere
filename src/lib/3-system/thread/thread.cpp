#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/memory/mmap_anonymous.hpp"
#include "3-system/memory/mprotect.hpp"
#include "3-system/memory/munmap.hpp"
#include "3-system/memory/page_size.hpp"
#include "3-system/memory/protection.hpp"
#include "3-system/process/impl/clone_onto.hpp"
#include "3-system/syscall/syscalls.hpp"
#include "3-system/thread/impl/thread_exit.hpp"
#include "3-system/thread/impl/tls.hpp"
#include "3-system/thread/thread.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)
#	include <linux/futex.h>
#	include <linux/sched.h>
#endif


namespace ca {


	namespace sys {


		namespace impl {

			/* @DOC The top of a thread's stack. _id is the kernel's: set before clone returns,
			   cleared when the thread has left user space for good. */
			struct thread_control final {
					using self = ca::sys::impl::thread_control;
					ca::atomic<ca::u32> _id;
					void (*_entry)(void*);
					void* _argument;
			}; // struct thread_control

		} // namespace impl

	} // namespace sys

} // namespace ca

#if ca_platform(linux) && ca_architecture(x86_64)

static constexpr ca::isz THREAD_FLAGS = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM | CLONE_SETTLS | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID;
static constexpr ca::usz MAXIMUM_STACK = ca::usz{1U} << 47U;
static constexpr ca::usz TLS_OFFSET = 64U;

static_assert(sizeof(ca::sys::impl::thread_control) <= TLS_OFFSET);

static auto run(void* const argument) noexcept -> int {

	const ca::sys::impl::thread_control& control = *static_cast<ca::sys::impl::thread_control*>(argument);
	control._entry(control._argument);
	ca::sys::impl::finish_thread();
	return 0;
}

static auto stack_canary(void) noexcept -> ca::u64 {

	ca::u64 canary = 0U;
	asm("movq %%fs:0x28, %0" : "=r"(canary));
	return canary;
}

[[nodiscard]] auto ca::sys::thread::make(void (* const entry)(void*), void* const argument, const ca::usz stack_size) noexcept -> ca::result<self> {

	const ca::sys::impl::tls_image image = ca::sys::impl::program_tls;
	if (image._align == 0U) [[unlikely]]
		return ca::failure{ca::errc::not_supported};
	if (stack_size < MINIMUM_STACK_SIZE || stack_size > MAXIMUM_STACK) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	const ca::usz page = ca::sys::page_size();
	const ca::usz stack = (stack_size + page - 1U) & ~(page - 1U);
	const ca::usz top = (TLS_OFFSET + ca::sys::impl::tls_area_size(image) + page - 1U) & ~(page - 1U);
	auto mapping = ca::sys::mmap_anonymous(page + stack + top, ca::sys::protection::read | ca::sys::protection::write);
	if (!mapping) [[unlikely]]
		return mapping.propagate();
	auto guarded = ca::sys::mprotect(ca::span<ca::byte>{mapping->data(), page}, ca::sys::protection::none);
	if (!guarded) [[unlikely]] {
		static_cast<void>(ca::sys::munmap(*mapping));
		return guarded.propagate();
	}
	ca::byte* const stack_top = mapping->data() + page + stack;
	ca::sys::impl::thread_control* const control = ca::construct_at(reinterpret_cast<ca::sys::impl::thread_control*>(stack_top));
	   control->_entry = entry;
	control->_argument = argument;
	const ca::u64 pointer = ca::sys::impl::install_tls(image, reinterpret_cast<char*>(stack_top) + TLS_OFFSET, stack_canary());
	ca::i32* const word = reinterpret_cast<ca::i32*>(&control->_id);
	const auto child = ca::sys::impl::clone_onto(THREAD_FLAGS, stack_top, word, word, pointer, &run, control);
	if (!child) [[unlikely]] {
		static_cast<void>(ca::sys::munmap(*mapping));
		return child.propagate();
	}
	return self{*mapping, control};
}

auto ca::sys::thread::id(void) const noexcept -> ca::i32 {
	return static_cast<ca::i32>(_control->_id.load<ca::memory_order::acquire>());
}

/* Not futex_wait: the kernel's wake for CLONE_CHILD_CLEARTID is a shared one, which a private
   waiter never sees. */
[[nodiscard]] auto ca::sys::thread::join(void) noexcept -> ca::result<void> {

	if (_control == nullptr) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	for (;;) {
		const ca::u32 id = _control->_id.load<ca::memory_order::acquire>();
		if (id == 0U)
			break;
		static_cast<void>(ca::syscall::futex(reinterpret_cast<ca::u32*>(&_control->_id), FUTEX_WAIT, id, nullptr, nullptr, 0));
	}
	_control = nullptr;
	return ca::sys::munmap(_mapping);
}

#else

[[nodiscard]] auto ca::sys::thread::make(void (* const)(void*), void* const, const ca::usz) noexcept -> ca::result<self> {
	return ca::failure{ca::errc::not_supported};
}

auto ca::sys::thread::id(void) const noexcept -> ca::i32 {
	return 0;
}

[[nodiscard]] auto ca::sys::thread::join(void) noexcept -> ca::result<void> {
	return ca::failure{ca::errc::invalid_argument};
}

#endif

[[nodiscard]] auto ca::sys::thread::cleanup(void) noexcept -> ca::result<void> {

	if (_control == nullptr)
		return ca::result<void>{};
	return join();
}
