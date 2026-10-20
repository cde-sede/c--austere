#ifndef ca_primitives_debug_recovery_hpp
#define ca_primitives_debug_recovery_hpp

#include "1-primitives/debug/frame_address.hpp"
#include "1-primitives/debug/source_location.hpp"
#include "1-primitives/debug/verdict.hpp"
#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- RECOVERY -----------------------------------------------------------

	namespace impl {

		/* @DOC What a trap carries while contexts decide on it: its reason, where it was
		   (result::raise records its caller; line 0 when unknown), the error code and trace number
		   of a failed result accessed (0 when none), the error object itself when it is not a
		   ca::error (_payload, of type _type, a type_key) and how to clean it up when it is owned
		   and no context takes it (_discard), whether ca::ask raised it (_resumable), and whether
		   _code and _trace already carry the reason (_traced, set by the first context that needs
		   the trap as a ca::error, so the reason is recorded once). _frame is the frame record of
		   the ca function that raised the trap, whose return address is the trap site: backtraces
		   start there. _payload and _frame point into the trapping frames: valid until the trap
		   unwinds. */
		struct trap_info final {

				using self = ca::impl::trap_info;

				const char* _reason;
				const void* _frame;
				ca::source_location _where;
				int _code;
				ca::u32 _trace;
				void* _payload;
				const void* _type;
				auto (*_discard)(void* payload) noexcept -> void;
				bool _resumable;
				bool _traced;

		}; // struct trap_info

		/* @DOC A context as a trap sees it: the jump buffer of __builtin_setjmp, the enclosing
		   point, the context's own state (_context) for _handles, which decides on a trap
		   (nullptr: takes every trap, passes every ca::ask), and _unwind, which runs the context's leave when a trap
		   passes it for an outer context. _take runs on the point a trap unwinds to, before it
		   does: it takes the trap's error into _sink when of the context's type (by move when it
		   cannot be copied), setting _accepted, else records the trap in the thread's trace.
		   _trap is what the trap carried, copied into the point it unwinds to.
		   Points form a per-thread list, innermost in current_recovery; ca::context_manager
		   pushes and pops them. */
		struct recovery final {

				using self = ca::impl::recovery;

				void* _buffer[5];
				ca::impl::recovery* _parent;
				void* _context;
				auto (*_handles)(void* context, const ca::impl::trap_info& info) noexcept -> ca::verdict;
				auto (*_unwind)(void* context, ca::impl::trap_info& info) noexcept -> void;
				void* _sink;
				auto (*_take)(ca::impl::recovery* point, ca::impl::trap_info& info) noexcept -> void;
				bool _accepted;
				ca::impl::trap_info _trap;

		}; // struct recovery

		/* @DOC The thread's innermost recovery point, nullptr outside every context, and while
		   contexts decide on a trap or leave on its way: a trap there stops the program. */
		inline constinit thread_local ca::impl::recovery* current_recovery{nullptr};

		/* @DOC Resumes at point, in the function that set it with __builtin_setjmp. A function of
		   its own: __builtin_longjmp may not be called from the function that called
		   __builtin_setjmp. */
		[[noreturn, gnu::cold, gnu::noinline]] inline auto recover(ca::impl::recovery* const point) noexcept -> void {
			__builtin_longjmp(point->_buffer, 1);
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto trap_resumed(void) noexcept -> void {
			__builtin_trap();
		}

		/* @DOC Both phases of a trap. Search: from the innermost point outward, each context's
		   _handles decides, at the trap site. Cleanup: every context passed runs its leave
		   (_unwind), innermost first, the target takes the error (_accept), and the trap unwinds
		   to it. Returns true when a context resumed info (resumable only: resuming anything
		   else stops the program), false when none took it; an owned error no context took is
		   cleaned up (_discard) first. The target's _take runs while the trapping frames are still
		   alive. */
		[[gnu::cold, gnu::noinline]] inline auto raise(ca::impl::trap_info& info) noexcept -> bool {

			ca::impl::recovery* const innermost = ca::impl::current_recovery;
			ca::impl::recovery* target = nullptr;
			ca::impl::current_recovery = nullptr;
			for (ca::impl::recovery* point = innermost; point != nullptr; point = point->_parent) {
				const ca::verdict verdict = point->_handles != nullptr ? point->_handles(point->_context, info)
				                          : info._resumable        ? ca::verdict::pass
				                                                   : ca::verdict::take;
				if (verdict == ca::verdict::take) {
					target = point;
					break;
				}
				if (verdict == ca::verdict::resume) {
					if (!info._resumable)
						ca::impl::trap_resumed();
					ca::impl::current_recovery = innermost;
					return true;
				}
			}
			if (target == nullptr) {
				if (info._discard != nullptr)
					info._discard(info._payload);
				ca::impl::current_recovery = innermost;
				return false;
			}
			for (ca::impl::recovery* point = innermost; point != target; point = point->_parent)
				if (point->_unwind != nullptr)
					point->_unwind(point->_context, info);
			if (target->_take != nullptr)
				target->_take(target, info);
			if (info._discard != nullptr && !target->_accepted)
				info._discard(info._payload);
			target->_trap = info;
			ca::impl::recover(target);
		}

		/* @DOC A trap that cannot resume: raise, then stop the program when no context took it. */
		[[noreturn, gnu::cold, gnu::noinline]] inline auto stop(ca::impl::trap_info& info) noexcept -> void {

			static_cast<void>(ca::impl::raise(info));
			__builtin_trap();
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto stop(const char* const reason) noexcept -> void {

			ca::impl::trap_info info{};
			info._reason = reason;
			 info._frame = ca::impl::frame_address();
			ca::impl::stop(info);
		}

	} // namespace impl

} // namespace ca

#endif // ca_primitives_debug_recovery_hpp
