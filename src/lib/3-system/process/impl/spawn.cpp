#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/isz.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/string_length.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/environment/environ.hpp"
#include "3-system/environment/impl/variables.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/memory/free.hpp"
#include "3-system/memory/mallocarray.hpp"
#include "3-system/memory/mmap_anonymous.hpp"
#include "3-system/memory/munmap.hpp"
#include "3-system/memory/protection.hpp"
#include "3-system/memory/reallocarray.hpp"
#include "3-system/process/impl/spawn.hpp"
#include "3-system/process/spawn_actions.hpp"
#include "3-system/syscall/impl/enter.hpp"
#include "3-system/syscall/syscalls.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

namespace ca {


	namespace sys {


		namespace impl {

			/* @DOC What the child of spawn_process gets: its work and where to report a failure. */
			struct spawn_job final {
					using self = ca::sys::impl::spawn_job;
					const char* _path;
					bool _search;
					const ca::sys::impl::spawn_list* _actions;
					char* const* _argv;
					char* const* _envp;
					ca::u64 _mask;
					int _error;
			}; // struct spawn_job

			struct spawn_list_layout final {
					using self = ca::sys::impl::spawn_list_layout;
					static_assert(sizeof(ca::sys::impl::spawn_list) <= ca::sys::impl::spawn_actions_size);
					static_assert(alignof(ca::sys::impl::spawn_list) <= alignof(void*));
			}; // struct spawn_list_layout

		} // namespace impl

	} // namespace sys

} // namespace ca

static constexpr int CLOSE = 0;
static constexpr int DUP2 = 1;
static constexpr int OPEN = 2;
static constexpr ca::usz CHILD_STACK = 65536U;
static constexpr ca::isz CLONE_VM = 0x100;
static constexpr ca::isz CLONE_VFORK = 0x4000;
static constexpr ca::isz SIGCHLD = 17;

/* A syscall for the vfork child: no error recorded, which would write the parent's
   thread_local state. */
template <typename... As>
[[gnu::always_inline]] static inline auto raw(const ca::isz number, const As... arguments) noexcept -> ca::isz {
	return ca::syscall::impl::enter(number, ca::syscall::impl::word(arguments)...);
}

static auto failed(const ca::isz result) noexcept -> bool {
	return result < 0 && result > -4096;
}

/* clone onto stack (its top), child running entry(argument) then exiting with its result. */
extern "C" auto ca_sys_spawn_clone(ca::isz flags, void* stack, int (*entry)(void*), void* argument) noexcept -> ca::isz; // ns-leak: ignore

asm(".text\n" // norm: ignore
	".p2align 4\n"
	"ca_sys_spawn_clone:\n"
	"\tsubq $16, %rsi\n"
	"\tmovq %rcx, 8(%rsi)\n"
	"\tmovq %rdx, (%rsi)\n"
	"\txorl %edx, %edx\n"
	"\txorl %r10d, %r10d\n"
	"\txorl %r8d, %r8d\n"
	"\tmovl $56, %eax\n"
	"\tsyscall\n"
	"\ttestq %rax, %rax\n"
	"\tjnz 1f\n"
	"\txorl %ebp, %ebp\n"
	"\tpopq %rax\n"
	"\tpopq %rdi\n"
	"\tcall *%rax\n"
	"\tmovl %eax, %edi\n"
	"\tmovl $60, %eax\n"
	"\tsyscall\n"
	"\thlt\n"
	"1:\n"
	"\tret\n");

static auto searches(const char* const file) noexcept -> bool {

	for (const char* at = file; *at != '\0'; ++at) {
		if (*at == '/')
			return false;
	}
	return true;
}

/* The child of spawn_process; raw syscalls only. */
static auto spawned(void* const argument) noexcept -> int {

	ca::sys::impl::spawn_job& job = *static_cast<ca::sys::impl::spawn_job*>(argument);
	for (int signal = 1; signal <= 64; ++signal) {
		ca::u64 old[4] = {};
		if (failed(raw(ca::syscall::number::rt_sigaction, signal, 0, old, 8)))
			continue;
		if (old[0] > 1U) {
			const ca::u64 action[4] = {0U, 0U, 0U, 0U};
			static_cast<void>(raw(ca::syscall::number::rt_sigaction, signal, action, 0, 8));
		}
	}
	static_cast<void>(raw(ca::syscall::number::rt_sigprocmask, 2, &job._mask, 0, 8));
	if (job._actions != nullptr) {
		for (int i = 0; i < job._actions->_used; ++i) {
			const ca::sys::impl::spawn_action& action = job._actions->_actions[i];
			ca::isz result = 0;
			if (action._kind == CLOSE) {
				result = raw(ca::syscall::number::close, action._fd);
				if (result == -9)
					result = 0;
			} else if (action._kind == DUP2) {
				result = raw(ca::syscall::number::dup2, action._fd, action._target);
				if (!failed(result) && action._fd == action._target) {
					const ca::isz flags = raw(ca::syscall::number::fcntl, action._fd, 1, 0);
					result = failed(flags) ? flags : raw(ca::syscall::number::fcntl, action._fd, 2, flags & ~ca::isz{1});
				}
			} else {
				const ca::isz fd = raw(ca::syscall::number::openat, -100, action._path, action._flags, action._mode);
				result = fd;
				if (!failed(fd) && fd != action._fd) {
					result = raw(ca::syscall::number::dup2, fd, action._fd);
					static_cast<void>(raw(ca::syscall::number::close, fd));
				}
			}
			if (failed(result)) {
				job._error = static_cast<int>(-result);
				return 127;
			}
		}
	}
	if (job._search)
		job._error = ca::sys::impl::execute_searching(job._path, job._argv, job._envp).code();
	else
		job._error = static_cast<int>(-raw(ca::syscall::number::execve, job._path, job._argv, job._envp));
	return 127;
}

[[nodiscard]] static auto add(ca::sys::impl::spawn_list& list, const ca::sys::impl::spawn_action& action) noexcept -> ca::result<void> {

	if (list._used == list._allocated) {
		const int grown = list._allocated == 0 ? 8 : list._allocated * 2;
		const auto block = ca::sys::reallocarray(list._actions, static_cast<ca::usz>(grown), sizeof(ca::sys::impl::spawn_action));
		if (!block) [[unlikely]]
			return ca::failure{ca::errc::out_of_memory};
		  list._actions = static_cast<ca::sys::impl::spawn_action*>(*block);
		list._allocated = grown;
	}
	list._actions[list._used++] = action;
	return ca::result<void>{};
}

auto ca::sys::impl::spawn_list_init(ca::sys::impl::spawn_list& list) noexcept -> void {
	list = ca::sys::impl::spawn_list{0, 0, nullptr};
}

auto ca::sys::impl::spawn_list_destroy(ca::sys::impl::spawn_list& list) noexcept -> void {

	for (int i = 0; i < list._used; ++i)
		ca::sys::free(list._actions[i]._path);
	ca::sys::free(list._actions);
	list = ca::sys::impl::spawn_list{0, 0, nullptr};
}

[[nodiscard]] auto ca::sys::impl::spawn_list_add_close(ca::sys::impl::spawn_list& list, const int fd) noexcept -> ca::result<void> {

	if (fd < 0) [[unlikely]]
		return ca::failure{ca::errc::bad_descriptor};
	return add(list, {CLOSE, fd, 0, 0, 0U, nullptr});
}

[[nodiscard]] auto ca::sys::impl::spawn_list_add_dup2(ca::sys::impl::spawn_list& list, const int fd, const int target) noexcept -> ca::result<void> {

	if (fd < 0 || target < 0) [[unlikely]]
		return ca::failure{ca::errc::bad_descriptor};
	return add(list, {DUP2, fd, target, 0, 0U, nullptr});
}

[[nodiscard]] auto ca::sys::impl::spawn_list_add_open(ca::sys::impl::spawn_list& list, const int fd, const char* const path, const int flags, const ca::u32 mode) noexcept -> ca::result<void> {

	if (fd < 0) [[unlikely]]
		return ca::failure{ca::errc::bad_descriptor};
	const ca::usz length = ca::string_length(path);
	const auto block = ca::sys::mallocarray(length + 1U, 1U);
	if (!block) [[unlikely]]
		return ca::failure{ca::errc::out_of_memory};
	char* const copy = static_cast<char*>(*block);
	ca::copy_bytes(copy, path, length + 1U);
	auto added = add(list, {OPEN, fd, 0, flags, mode, copy});
	if (!added) [[unlikely]]
		ca::sys::free(copy);
	return added;
}

[[nodiscard]] auto ca::sys::impl::execute_searching(const char* const file, char* const* const argv, char* const* const envp) noexcept -> ca::error {

	if (!searches(file))
		return ca::error{static_cast<int>(-raw(ca::syscall::number::execve, file, argv, envp))};
	const char* const from_environment = ca::sys::impl::find_variable("PATH");
	const char* at = from_environment != nullptr ? from_environment : "/bin:/usr/bin";
	ca::isz result = -2;
	bool denied = false;
	char candidate[4096];
	const ca::usz name = ca::string_length(file);
	for (;;) {
		const char* end = at;
		while (*end != '\0' && *end != ':')
			++end;
		ca::usz length = static_cast<ca::usz>(end - at);
		if (length + name + 3U <= sizeof(candidate)) {
			ca::copy_bytes(candidate, at, length);
			if (length == 0U)
				candidate[length++] = '.';
			candidate[length++] = '/';
			ca::copy_bytes(candidate + length, file, name + 1U);
			result = raw(ca::syscall::number::execve, candidate, argv, envp);
			if (result == -13)
				denied = true;
			else if (result != -2 && result != -20)
				break;
		}
		if (*end == '\0')
			break;
		at = end + 1;
	}
	if (denied && (result == -2 || result == -20))
		result = -13;
	return ca::error{static_cast<int>(-result)};
}

[[nodiscard]] auto ca::sys::impl::spawn_process(const char* const path, const bool search, const ca::sys::impl::spawn_list* const actions, char* const* const argv, char* const* const envp) noexcept -> ca::result<ca::i32> {

	ca::sys::impl::spawn_job job{path, search && searches(path), actions, argv, envp != nullptr ? envp : ca::sys::environ(), 0U, 0};
	const auto stack = ca::sys::mmap_anonymous(CHILD_STACK, ca::sys::protection::read | ca::sys::protection::write);
	if (!stack) [[unlikely]]
		return ca::failure{stack.error()};
	const ca::u64 all = ~ca::u64{0};
	static_cast<void>(ca::syscall::rt_sigprocmask(0, &all, &job._mask, 8));
	const ca::isz child = ca_sys_spawn_clone(CLONE_VM | CLONE_VFORK | SIGCHLD, stack->data() + CHILD_STACK, &spawned, &job);
	static_cast<void>(ca::syscall::rt_sigprocmask(2, &job._mask, nullptr, 8));
	static_cast<void>(ca::sys::munmap(*stack));
	if (failed(child)) [[unlikely]]
		return ca::failure{ca::error{static_cast<int>(-child)}};
	if (job._error != 0) {
		int status = 0;
		static_cast<void>(ca::syscall::wait4(child, &status, 0, nullptr));
		return ca::failure{ca::error{job._error}};
	}
	return static_cast<ca::i32>(child);
}

[[nodiscard]] auto ca::sys::impl::fork_process(void) noexcept -> ca::result<ca::i32> {
	return ca::syscall::clone(SIGCHLD, 0, 0, 0, 0).transform([](const ca::isz child) { return static_cast<ca::i32>(child); });
}

#endif
