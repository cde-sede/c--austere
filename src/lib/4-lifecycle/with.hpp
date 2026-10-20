#ifndef ca_lifecycle_with_hpp
#define ca_lifecycle_with_hpp

#include "1-primitives/debug/verdict.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/utility/forward.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/handler.hpp"
#include "4-lifecycle/impl/run_context.hpp"
#include "4-lifecycle/outcome.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- WITH ---------------------------------------------------------------

	namespace impl {

		template <typename T>
		inline constexpr bool is_handler = false;

		template <typename F>
		inline constexpr bool is_handler<ca::handler<F>> = true;

		template <typename E, typename Handles, typename Enter, typename Leave, typename... Args>
		[[nodiscard]] auto with(Handles& handles, Enter& enter, Leave& leave, Args&... args) noexcept -> ca::result<void, E> {

			auto entered = [&enter, &args...](void) noexcept -> decltype(auto) {
				return enter(ca::forward<Args>(args)...);
			};
			auto left = [&leave](const E* const error, const ca::outcome outcome) noexcept -> void {
				if constexpr (requires { leave(error, outcome); })
					leave(error, outcome);
				else
					leave(error);
			};
			return ca::impl::run_context<E>(handles, entered, left);
		}

	} // namespace impl


	/* @DOC ca::context_manager from callables, the state in what they capture:
	   enter(args...) inside a new context, then leave(const E* error), error nullptr on success,
	   or leave(error, ca::outcome). Traps inside enter fail the context as with
	   ca::context_manager: `ca::with<ca::report<>>(enter, leave)` fails with a report. A leading
	   ca::handler is the context's handles: `ca::with(ca::handler{decide}, enter, leave)`;
	   without one, the context takes every trap. */
	template <typename E = ca::error, typename Enter, typename Leave, typename... Args>
		requires (!ca::impl::is_handler<ca::remove_cvref<Enter>>)
	[[nodiscard]] auto with(Enter&& enter, Leave&& leave, Args&&... args) noexcept -> ca::result<void, E> {

		ca::impl::takes_every_trap handles{};
		return ca::impl::with<E, ca::impl::takes_every_trap, Enter, Leave, Args...>(handles, enter, leave, args...);
	}

	template <typename E = ca::error, typename F, typename Enter, typename Leave, typename... Args>
	[[nodiscard]] auto with(ca::handler<F> handles, Enter&& enter, Leave&& leave, Args&&... args) noexcept -> ca::result<void, E> {
		return ca::impl::with<E, ca::handler<F>, Enter, Leave, Args...>(handles, enter, leave, args...);
	}

} // namespace ca

#endif // ca_lifecycle_with_hpp
