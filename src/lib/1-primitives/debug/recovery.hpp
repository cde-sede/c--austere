#ifndef ca_primitives_debug_recovery_hpp
#define ca_primitives_debug_recovery_hpp

#include "1-primitives/debug/source_location.hpp"
#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- RECOVERY -----------------------------------------------------------

	namespace impl {

		/* @DOC A point ca::trap returns to instead of stopping the program: the jump buffer of
		   __builtin_setjmp, the enclosing point, and what the trap left: its reason, where it was
		   (result::raise records its caller; line 0 when unknown), the error code
		   and trace number of a failed result accessed (0 when none), and whether _accept took that
		   error whole. _accept(_sink, error, type_key) takes an error of the context's type (by
		   move when it cannot be copied) and answers whether it did. Points form a per-thread
		   list, innermost in current_recovery; ca::context_manager pushes and pops them. */
		struct recovery final {

				using self = ca::impl::recovery;

				void* _buffer[5];
				ca::impl::recovery* _parent;
				const char* _reason;
				ca::source_location _where;
				void* _sink;
				auto (*_accept)(void* sink, void* error, const void* type) noexcept -> bool;
				bool _accepted;
				int _code;
				ca::u32 _trace;

		}; // struct recovery

		/* @DOC The thread's innermost recovery point, nullptr outside every context. */
		inline constinit thread_local ca::impl::recovery* current_recovery{nullptr};

		/* @DOC Resumes at point, in the function that set it with __builtin_setjmp. A function of
		   its own: __builtin_longjmp may not be called from the function that called
		   __builtin_setjmp. */
		[[noreturn, gnu::cold, gnu::noinline]] inline auto recover(ca::impl::recovery* const point) noexcept -> void {
			__builtin_longjmp(point->_buffer, 1);
		}

	} // namespace impl

} // namespace ca

#endif // ca_primitives_debug_recovery_hpp
