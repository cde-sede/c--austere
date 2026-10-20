#ifndef ca_lifecycle_handler_hpp
#define ca_lifecycle_handler_hpp

#include "1-primitives/debug/verdict.hpp"
#include "1-primitives/utility/move.hpp"
#include "4-lifecycle/trapped.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- HANDLER ------------------------------------------------------------

	/* @DOC Marks a callable as the handles of a ca::with context:
	   `ca::with(ca::handler{decide}, enter, leave)`, decide(const ca::trapped&) -> ca::verdict. */
	template <typename F>
	class handler final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::handler<F>;


			// -- private members --------------------------------------------

			F _decide;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr explicit handler(F decide) noexcept
			: _decide{ca::move(decide)} {
			}


			// -- methods ----------------------------------------------------

			[[nodiscard]] constexpr auto operator()(const ca::trapped& trap) noexcept -> ca::verdict {
				return _decide(trap);
			}

	}; // class handler

} // namespace ca

#endif // ca_lifecycle_handler_hpp
