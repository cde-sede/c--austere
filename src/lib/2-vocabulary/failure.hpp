#ifndef ca_vocabulary_failure_hpp
#define ca_vocabulary_failure_hpp

#include "1-primitives/utility/move.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FAILURE ------------------------------------------------------------

	/* @DOC An error on its way into a result: `return ca::failure{e};`. */
	template <typename E>
	class failure final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::failure<E>;


			// -- private members --------------------------------------------

			E _error;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr explicit failure(const E& error) noexcept
			: _error{error} {
			}

			constexpr explicit failure(E&& error) noexcept
			: _error{ca::move(error)} {
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto error(void) & noexcept -> E& {
				return _error;
			}

			[[nodiscard]] constexpr auto error(void) const& noexcept -> const E& {
				return _error;
			}

			[[nodiscard]] constexpr auto error(void) && noexcept -> E&& {
				return ca::move(_error);
			}

	}; // class failure

} // namespace ca

#endif // ca_vocabulary_failure_hpp
