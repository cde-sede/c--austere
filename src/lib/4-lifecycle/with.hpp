#ifndef ca_lifecycle_with_hpp
#define ca_lifecycle_with_hpp

#include "1-primitives/utility/forward.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/impl/run_context.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- WITH ---------------------------------------------------------------

	/* @DOC ca::context_manager from two callables, the state in what they capture:
	   enter(args...) inside a new context, then leave(const E* error), error nullptr on success.
	   Traps inside enter fail the context as with ca::context_manager:
	   `ca::with<ca::report<>>(enter, leave)` fails with a report. */
	template <typename E = ca::error, typename Enter, typename Leave, typename... Args>
	[[nodiscard]] auto with(Enter&& enter, Leave&& leave, Args&&... args) noexcept -> ca::result<void, E> {

		auto entered = [&enter, &args...](void) noexcept -> decltype(auto) {
			return enter(ca::forward<Args>(args)...);
		};
		auto left = [&leave](const E* const error) noexcept -> void {
			leave(error);
		};
		return ca::impl::run_context<E>(entered, left);
	}

} // namespace ca

#endif // ca_lifecycle_with_hpp
