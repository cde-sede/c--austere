#ifndef ca_system_process_impl_spawn_hpp
#define ca_system_process_impl_spawn_hpp

#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SPAWN ----------------------------------------------------------

		/* @DOC Processes without libc, on raw syscalls: what libca_runtime's fork, execvp,
		   posix_spawn, posix_spawnp and posix_spawn_file_actions_* are. In libc mode
		   ca::sys::fork and the others stay libc's (its fork runs atfork handlers and keeps its
		   own state right). Linux x86-64 only.

		   spawn_process as glibc's posix_spawn: the parent blocks every signal and clones with
		   CLONE_VM and CLONE_VFORK (no copy of the address space; the parent sleeps until the
		   child execs or exits) onto a stack of its own. The child sets caught signals back to
		   their default (a handler of the parent must not run in a child sharing its memory),
		   restores the mask, joins its process group when asked, applies the file actions, execs; a failure leaves its error in
		   shared memory and exits 127, and the parent then reaps the child and returns that
		   error. The child makes raw syscalls only: it shares the parent's memory and thread
		   pointer, so it must not touch thread_local state (sys::last_error). */
		namespace impl {

			/* @DOC One file action of a spawn: close fd, dup2(fd, target), or open path at fd. */
			struct spawn_action final {
					using self = ca::sys::impl::spawn_action;
					int _kind;
					int _fd;
					int _target;
					int _flags;
					ca::u32 _mode;
					char* _path;
			}; // struct spawn_action

			/* @DOC The file actions of a spawn, in the storage of a posix_spawn_file_actions_t
			   (glibc's is 80 bytes; this uses 16): a malloc'd array, each path a copy. */
			struct spawn_list final {
					using self = ca::sys::impl::spawn_list;
					int _allocated;
					int _used;
					ca::sys::impl::spawn_action* _actions;
			}; // struct spawn_list

			/* @DOC POSIX_SPAWN_SETPGROUP: the child joins _process_group. */
			inline constexpr short spawn_sets_process_group = 0x2;

			/* @DOC The attributes of a spawn, in the storage of a posix_spawnattr_t: the
			   POSIX_SPAWN_* flags set, and the group for spawn_sets_process_group. */
			struct spawn_attributes final {
					using self = ca::sys::impl::spawn_attributes;
					short _flags;
					ca::i32 _process_group;
			}; // struct spawn_attributes

			/* @DOC An empty list. */
			auto spawn_list_init(ca::sys::impl::spawn_list& list) noexcept -> void;

			/* @DOC Frees the list and its paths, leaving it empty. */
			auto spawn_list_destroy(ca::sys::impl::spawn_list& list) noexcept -> void;

			/* @DOC Adds closing fd. errc::bad_descriptor for a negative one, errc::out_of_memory. */
			[[nodiscard]] auto spawn_list_add_close(ca::sys::impl::spawn_list& list, int fd) noexcept -> ca::result<void>;

			/* @DOC Adds dup2(fd, target), which clears close-on-exec when fd is target. Errors as
			   spawn_list_add_close. */
			[[nodiscard]] auto spawn_list_add_dup2(ca::sys::impl::spawn_list& list, int fd, int target) noexcept -> ca::result<void>;

			/* @DOC Adds opening path (copied) with flags and mode as fd. Errors as
			   spawn_list_add_close. */
			[[nodiscard]] auto spawn_list_add_open(ca::sys::impl::spawn_list& list, int fd, const char* path, int flags, ca::u32 mode) noexcept -> ca::result<void>;

			/* @DOC Starts path as a child with argv and envp (null: the environment), looked up in
			   $PATH when search and it has no '/', the attributes applied first, then the actions
			   (null for none of either); returns its id. The error of what failed in the child,
			   when something did. */
			[[nodiscard]] auto spawn_process(const char* path, bool search, const ca::sys::impl::spawn_list* actions, const ca::sys::impl::spawn_attributes* attributes, char* const* argv, char* const* envp) noexcept -> ca::result<ca::i32>;

			/* @DOC execve of file with argv and envp, looked up in $PATH (/bin:/usr/bin when unset;
			   an empty entry is ".") when it has no '/'. Returns only on failure, its error:
			   errc::access_denied when some candidate was denied and none found. Raw syscalls
			   only, so a vfork child may call it. */
			[[nodiscard]] auto execute_searching(const char* file, char* const* argv, char* const* envp) noexcept -> ca::error;

			/* @DOC A copy of the process (clone with SIGCHLD only): the child's id, 0 in the child. */
			[[nodiscard]] auto fork_process(void) noexcept -> ca::result<ca::i32>;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_process_impl_spawn_hpp
