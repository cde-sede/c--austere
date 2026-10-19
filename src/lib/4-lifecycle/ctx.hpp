#ifndef ca_lifecycle_ctx_hpp
#define ca_lifecycle_ctx_hpp

#include "1-primitives/traits/is_default_constructible.hpp"
#include "1-primitives/utility/forward.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/context_manager.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- CTX ----------------------------------------------------------------

	/* @DOC A context run on a state of its own: ca::ctx<Self>::run(args...) builds a Self (a
	   ca::context_manager, default-constructible) and runs it, Self{}.run(args...). */
	template <typename Self>
	class ctx final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::ctx<Self>;


		public:

			// -- public lifecycle -------------------------------------------

			ctx(void) = delete;


			// -- methods ----------------------------------------------------

			/* @DOC enter(args...) on a new Self, then leave. Success, or the error leave saw. */
			template <typename... Args>
			[[nodiscard]] static auto run(Args&&... args) noexcept -> ca::result<void, typename Self::error_type> {

				static_assert(ca::is_default_constructible<Self>, "ca::ctx: Self must be default-constructible");

				Self state{};
				return state.run(ca::forward<Args>(args)...);
			}

	}; // class ctx

} // namespace ca

#endif // ca_lifecycle_ctx_hpp
