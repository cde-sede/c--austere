#ifndef ca_system_sync_once_hpp
#define ca_system_sync_once_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/traits/invoke_result.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/is_void.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ONCE ---------------------------------------------------------------

	/* @DOC Runs a function once among every thread calling call: the first runs it, the others
	   sleep until it is done. A function returning a failed ca::result<void> leaves it not done,
	   so the next call runs it again. A trap inside the function, or call from the function
	   itself, leaves the other callers waiting forever. Constant-initialized, trivially
	   destructible, neither copyable nor movable. */
	class once final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::once;


			// -- private members --------------------------------------------

			/* 0 new, 1 running, 2 running with sleepers, 3 done. */
			ca::atomic<ca::u32> _state;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr once(void) noexcept
			: _state{0U} {
			}

			once(const self&) = delete;


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;


			// -- accessors --------------------------------------------------

			[[nodiscard]] auto is_done(void) const noexcept -> bool {
				return _state.load<ca::memory_order::acquire>() == 3U;
			}


			// -- methods ----------------------------------------------------

			/* @DOC Runs f (void or ca::result<void>) unless it already ran; f's failure when it ran
			   and failed. */
			template <typename F>
			[[nodiscard]] auto call(F&& f) noexcept -> ca::result<void> {

				using R = ca::invoke_result<F>;
				static_assert(ca::is_void<R> || ca::is_same<R, ca::result<void>>, "ca::once::call: f must return void or ca::result<void>");
				if (is_done() || !_begin()) [[likely]]
					return ca::result<void>{};
				if constexpr (ca::is_void<R>) {
					f();
					_finish(true);
					return ca::result<void>{};
				}
				else {
					ca::result<void> ran = f();
					_finish(ran.ok());
					return ran;
				}
			}


		private:

			// -- private methods --------------------------------------------

			/* @DOC true: the caller runs the function; false: it is done. */
			[[nodiscard]] auto _begin(void) noexcept -> bool;

			auto _finish(bool done) noexcept -> void;

	}; // class once

} // namespace ca

#endif // ca_system_sync_once_hpp
