/* fork, execvp, posix_spawn, posix_spawnp and the posix_spawn_file_actions_* functions for
   programs linked without libc (see start_x86_64.cpp).

   posix_spawn as glibc does it: the parent blocks every signal and clones with CLONE_VM and
   CLONE_VFORK (no copy of the address space; the parent sleeps until the child execs or exits)
   onto a stack of its own. The child sets caught signals back to their default (a handler of
   the parent must not run in a child sharing its memory), restores the mask, applies the file
   actions, execs; a failure leaves its errno in shared memory and exits 127, and the parent
   then reaps the child and returns that error. The child makes raw syscalls only: it shares
   the parent's memory and thread pointer, so it must not touch thread_local state.

   posix_spawn_file_actions_t (80 bytes in glibc) holds {allocated, used, actions}: actions a
   malloc'd array of {kind, fd, target, flags, mode, path}, path a copy. */

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/isz.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "3-system/syscall/impl/enter.hpp"
#include "3-system/syscall/syscalls.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

extern "C" auto __errno_location(void) noexcept -> int*;
extern "C" auto malloc(ca::usz size) noexcept -> void*;
extern "C" auto realloc(void* block, ca::usz size) noexcept -> void*;
extern "C" auto free(void* block) noexcept -> void;
extern "C" auto getenv(const char* name) noexcept -> char*;
extern "C" char** environ;

namespace ca {


	namespace runtime {


		namespace impl {

			/* @DOC One file action of a spawn: close fd, dup2(fd, target), or open path at fd. */
			struct spawn_action final {
					using self = ca::runtime::impl::spawn_action;
					int _kind;
					int _fd;
					int _target;
					int _flags;
					ca::u32 _mode;
					char* _path;
			}; // struct spawn_action

			/* @DOC What posix_spawn_file_actions_t holds here (its first 16 of 80 bytes). */
			struct spawn_list final {
					using self = ca::runtime::impl::spawn_list;
					int _allocated;
					int _used;
					ca::runtime::impl::spawn_action* _actions;
			}; // struct spawn_list

			/* @DOC What the child of posix_spawn gets: its work and where to report a failure. */
			struct spawn_job final {
					using self = ca::runtime::impl::spawn_job;
					const char* _path;
					bool _search;
					const ca::runtime::impl::spawn_list* _actions;
					char* const* _argv;
					char* const* _envp;
					const char* _search_path;
					ca::u64 _mask;
					int _error;
			}; // struct spawn_job

		} // namespace impl

	} // namespace runtime

} // namespace ca

static constexpr int CLOSE = 0;
static constexpr int DUP2 = 1;
static constexpr int OPEN = 2;
static constexpr ca::usz CHILD_STACK = 65536U;
static constexpr ca::isz CLONE_VM = 0x100;
static constexpr ca::isz CLONE_VFORK = 0x4000;
static constexpr ca::isz SIGCHLD = 17;

template <typename... As>
[[gnu::always_inline]] static inline auto raw(const ca::isz number, const As... arguments) noexcept -> ca::isz {
	return ca::syscall::impl::enter(number, ca::syscall::impl::word(arguments)...);
}

static auto failed(const ca::isz result) noexcept -> bool {
	return result < 0 && result > -4096;
}

static auto length_of(const char* const text) noexcept -> ca::usz {

	ca::usz length = 0U;
	while (text[length] != '\0')
		++length;
	return length;
}

/* clone onto stack (its top), child running entry(argument) then exiting with its result. */
extern "C" auto ca_runtime_clone(ca::isz flags, void* stack, int (*entry)(void*), void* argument) noexcept -> ca::isz; // ns-leak: ignore

asm(".text\n" // norm: ignore
	".p2align 4\n"
	"ca_runtime_clone:\n"
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

/* execve of job's path, or of each candidate in its search path (an empty entry is ".");
   returns the errno when nothing ran: EACCES if some candidate was denied and none found. */
static auto execute(const ca::runtime::impl::spawn_job& job) noexcept -> int {

	if (!job._search)
		return static_cast<int>(-raw(ca::syscall::number::execve, job._path, job._argv, job._envp));
	ca::isz result = -2;
	bool denied = false;
	char candidate[4096];
	const ca::usz name = length_of(job._path);
	const char* at = job._search_path;
	for (;;) {
		const char* end = at;
		while (*end != '\0' && *end != ':')
			++end;
		ca::usz length = static_cast<ca::usz>(end - at);
		if (length + name + 3U <= sizeof(candidate)) {
			for (ca::usz k = 0U; k < length; ++k)
				candidate[k] = at[k];
			if (length == 0U)
				candidate[length++] = '.';
			candidate[length++] = '/';
			for (ca::usz k = 0U; k <= name; ++k)
				candidate[length + k] = job._path[k];
			result = raw(ca::syscall::number::execve, candidate, job._argv, job._envp);
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
	return static_cast<int>(-result);
}

/* The child of posix_spawn; raw syscalls only. */
static auto spawned(void* const argument) noexcept -> int {

	ca::runtime::impl::spawn_job& job = *static_cast<ca::runtime::impl::spawn_job*>(argument);
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
			const ca::runtime::impl::spawn_action& action = job._actions->_actions[i];
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
	job._error = execute(job);
	return 127;
}

static auto spawn(int* const pid, const char* const path, const bool search, const void* const actions, char* const* const argv, char* const* const envp) noexcept -> int {

	ca::runtime::impl::spawn_job job{path, search, static_cast<const ca::runtime::impl::spawn_list*>(actions), argv, envp != nullptr ? envp : environ, nullptr, 0U, 0};
	if (search) {
		for (const char* at = path; *at != '\0'; ++at) {
			if (*at == '/') {
				job._search = false;
				break;
			}
		}
		const char* const from_environment = getenv("PATH");
		job._search_path = from_environment != nullptr ? from_environment : "/bin:/usr/bin";
	}
	const ca::isz stack = raw(ca::syscall::number::mmap, 0, CHILD_STACK, 3, 0x20022, -1, 0);
	if (failed(stack)) [[unlikely]]
		return static_cast<int>(-stack);
	const ca::u64 all = ~ca::u64{0};
	static_cast<void>(raw(ca::syscall::number::rt_sigprocmask, 0, &all, &job._mask, 8));
	const ca::isz child = ca_runtime_clone(CLONE_VM | CLONE_VFORK | SIGCHLD, reinterpret_cast<char*>(stack) + CHILD_STACK, &spawned, &job);
	static_cast<void>(raw(ca::syscall::number::rt_sigprocmask, 2, &job._mask, 0, 8));
	static_cast<void>(raw(ca::syscall::number::munmap, stack, CHILD_STACK));
	if (failed(child)) [[unlikely]]
		return static_cast<int>(-child);
	if (job._error != 0) {
		int status = 0;
		static_cast<void>(raw(ca::syscall::number::wait4, child, &status, 0, 0));
		return job._error;
	}
	if (pid != nullptr)
		*pid = static_cast<int>(child);
	return 0;
}

static auto add(void* const list_storage, const ca::runtime::impl::spawn_action& action) noexcept -> int {

	ca::runtime::impl::spawn_list& list = *static_cast<ca::runtime::impl::spawn_list*>(list_storage);
	if (list._used == list._allocated) {
		const int grown = list._allocated == 0 ? 8 : list._allocated * 2;
		void* const block = realloc(list._actions, static_cast<ca::usz>(grown) * sizeof(ca::runtime::impl::spawn_action));
		if (block == nullptr) [[unlikely]]
			return 12;
		list._actions = static_cast<ca::runtime::impl::spawn_action*>(block);
		list._allocated = grown;
	}
	list._actions[list._used++] = action;
	return 0;
}

extern "C" {

	auto fork(void) noexcept -> int { // ns-leak: ignore

		const ca::isz child = raw(ca::syscall::number::clone, SIGCHLD, 0, 0, 0, 0);
		if (failed(child)) [[unlikely]] {
			*__errno_location() = static_cast<int>(-child);
			return -1;
		}
		return static_cast<int>(child);
	}

	auto execvp(const char* const file, char* const* const argv) noexcept -> int { // ns-leak: ignore

		ca::runtime::impl::spawn_job job{file, true, nullptr, argv, environ, nullptr, 0U, 0};
		for (const char* at = file; *at != '\0'; ++at) {
			if (*at == '/')
				job._search = false;
		}
		const char* const from_environment = getenv("PATH");
		job._search_path = from_environment != nullptr ? from_environment : "/bin:/usr/bin";
		*__errno_location() = execute(job);
		return -1;
	}

	auto posix_spawn(int* const pid, const char* const path, const void* const actions, const void*, char* const* const argv, char* const* const envp) noexcept -> int { // ns-leak: ignore
		return spawn(pid, path, false, actions, argv, envp);
	}

	auto posix_spawnp(int* const pid, const char* const file, const void* const actions, const void*, char* const* const argv, char* const* const envp) noexcept -> int { // ns-leak: ignore
		return spawn(pid, file, true, actions, argv, envp);
	}

	auto posix_spawn_file_actions_init(void* const storage) noexcept -> int { // ns-leak: ignore

		*static_cast<ca::runtime::impl::spawn_list*>(storage) = ca::runtime::impl::spawn_list{0, 0, nullptr};
		return 0;
	}

	auto posix_spawn_file_actions_destroy(void* const storage) noexcept -> int { // ns-leak: ignore

		ca::runtime::impl::spawn_list& list = *static_cast<ca::runtime::impl::spawn_list*>(storage);
		for (int i = 0; i < list._used; ++i)
			free(list._actions[i]._path);
		free(list._actions);
		list = ca::runtime::impl::spawn_list{0, 0, nullptr};
		return 0;
	}

	auto posix_spawn_file_actions_addclose(void* const storage, const int fd) noexcept -> int { // ns-leak: ignore
		return fd < 0 ? 9 : add(storage, {CLOSE, fd, 0, 0, 0U, nullptr});
	}

	auto posix_spawn_file_actions_adddup2(void* const storage, const int fd, const int target) noexcept -> int { // ns-leak: ignore
		return fd < 0 || target < 0 ? 9 : add(storage, {DUP2, fd, target, 0, 0U, nullptr});
	}

	auto posix_spawn_file_actions_addopen(void* const storage, const int fd, const char* const path, const int flags, const ca::u32 mode) noexcept -> int { // ns-leak: ignore

		if (fd < 0) [[unlikely]]
			return 9;
		const ca::usz length = length_of(path);
		char* const copy = static_cast<char*>(malloc(length + 1U));
		if (copy == nullptr) [[unlikely]]
			return 12;
		for (ca::usz i = 0U; i <= length; ++i)
			copy[i] = path[i];
		const int code = add(storage, {OPEN, fd, 0, flags, mode, copy});
		if (code != 0)
			free(copy);
		return code;
	}

} // extern "C"

#endif
