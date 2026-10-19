#ifndef ca_lifecycle_context_manager_hpp
#define ca_lifecycle_context_manager_hpp

#include "1-primitives/utility/forward.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/impl/run_context.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- CONTEXT MANAGER ----------------------------------------------------

	/* @DOC CRTP base of a context, Python's with statement without exceptions. Self holds the
	   state and provides `enter(args...) -> void` (or `-> result<void, E>`) and
	   `leave(const E* error) noexcept -> void`. self.run(args...) calls enter inside a new
	   context, then leave: error is nullptr on success, else what the context fails with, which
	   run returns. ca::ctx<Self>::run(args...) does the same on a new Self.

	   Every ca::trap reached inside enter leaves for the innermost context. Accessing a failed
	   result (`*r`, `r->`, `r.value()`, `r.raise()`) fails it with r's error: as it is when of
	   type E (moved out of r when E cannot be copied), else its ca::error (an error() of it)
	   made an E, its trace extended with the reason. Any other trap fails it with
	   errc::not_recoverable and the reason. E is ca::error or made from one (ca::report<A>, by
	   E::of). Frames skipped this way are never unwound, so their objects must be trivially
	   destructible (every ca owner is); what they acquired belongs in Self, for leave to
	   release. Contexts nest, per thread. */
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
				auto leave = [&state](const E* const error) noexcept -> void {
					state.leave(error);
				};
				return ca::impl::run_context<E>(enter, leave);
			}

	}; // class context_manager

} // namespace ca

#endif // ca_lifecycle_context_manager_hpp
