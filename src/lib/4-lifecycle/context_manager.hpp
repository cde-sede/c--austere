#ifndef ca_lifecycle_context_manager_hpp
#define ca_lifecycle_context_manager_hpp

#include "1-primitives/debug/verdict.hpp"
#include "1-primitives/utility/forward.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/impl/run_context.hpp"
#include "4-lifecycle/outcome.hpp"
#include "4-lifecycle/trapped.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- CONTEXT MANAGER ----------------------------------------------------

	/* @DOC CRTP base of a context, Python's with statement without exceptions. Self holds the
	   state and provides `enter(args...) -> void` (or `-> result<void, E>`) and
	   `leave(const E* error) noexcept -> void`, or `leave(const E* error, ca::outcome)` to know
	   how the context ended. self.run(args...) calls enter inside a new context, then leave:
	   error is nullptr on success, else what the context fails with, which run returns.
	   ca::ctx<Self>::run(args...) does the same on a new Self.

	   Every ca::trap reached inside enter goes to the contexts, innermost first. Self may
	   provide `handles(const ca::trapped&) noexcept -> ca::verdict`, called at the trap site
	   before anything unwinds: take the trap, pass it to an outer context, or resume a
	   ca::ask. Without handles, a context takes every trap and passes every ca::ask. A context passed over still runs
	   leave, with outcome passed, during the trap; its run never returns. handles, and leave
	   when passed, must not trap: that stops the program.

	   Accessing a failed result (`*r`, `r->`, `r.value()`, `r.raise()`) fails the context
	   taking it with r's error: as it is when of type E (moved out of r when E cannot be
	   copied), else its ca::error (an error() of it) made an E, its trace extended with the
	   reason. Any other trap fails it with errc::not_recoverable and the reason. E is ca::error
	   or made from one (ca::report<A>, by E::of). Frames skipped this way are never unwound, so
	   their objects must be trivially destructible (every ca owner is); what they acquired
	   belongs in Self, for leave to release. Contexts nest, per thread. */
	template <typename Self, typename E = ca::error> // final: ignore
	class context_manager {


		private:

			// -- private types ----------------------------------------------

			using self = ca::context_manager<Self, E>;


		public:

			// -- public types -----------------------------------------------

			using error_type = E;


			// -- methods ----------------------------------------------------

			/* @DOC enter(args...) on this object, then leave. Success, or the error leave saw. */
			template <typename... Args>
			[[nodiscard]] auto run(Args&&... args) noexcept -> ca::result<void, E> {

				Self& state = static_cast<Self&>(*this);
				auto enter = [&state, &args...](void) noexcept -> decltype(auto) {
					return state.enter(ca::forward<Args>(args)...);
				};
				auto leave = [&state](const E* const error, const ca::outcome outcome) noexcept -> void {
					ca::impl::leave_state(state, error, outcome);
				};
				if constexpr (requires (const ca::trapped& trap) { state.handles(trap); }) {
					auto handles = [&state](const ca::trapped& trap) noexcept -> ca::verdict {
						return state.handles(trap);
					};
					return ca::impl::run_context<E>(handles, enter, leave);
				}
				else {
					ca::impl::takes_every_trap handles{};
					return ca::impl::run_context<E>(handles, enter, leave);
				}
			}

	}; // class context_manager

} // namespace ca

#endif // ca_lifecycle_context_manager_hpp
