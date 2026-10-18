#ifndef ca_system_process_wait_status_hpp
#define ca_system_process_wait_status_hpp

#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- WAIT STATUS ----------------------------------------------------

		/* @DOC How a child process ended: an exit code, or the signal that killed it. */
		class wait_status final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::wait_status;


				// -- private members ----------------------------------------

				bool _exited;
				int _value;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr wait_status(const bool exited, const int value) noexcept
				: _exited{exited}, _value{value} {
				}


				// -- accessors ----------------------------------------------

				/* @DOC Whether the child exited (rather than being killed by a signal). */
				[[nodiscard]] constexpr auto exited(void) const noexcept -> bool {
					return _exited;
				}

				/* @DOC Exit code, 0..255, or nothing when killed by a signal. */
				[[nodiscard]] constexpr auto exit_code(void) const noexcept -> ca::optional<int> {

					if (!_exited)
						return ca::nullopt;
					return _value;
				}

				/* @DOC Signal that killed the child, or nothing when it exited. */
				[[nodiscard]] constexpr auto signal(void) const noexcept -> ca::optional<int> {

					if (_exited)
						return ca::nullopt;
					return _value;
				}

				/* @DOC Whether the child exited with code 0. */
				[[nodiscard]] constexpr auto succeeded(void) const noexcept -> bool {
					return _exited && _value == 0;
				}

		}; // class wait_status

	} // namespace sys

} // namespace ca

#endif // ca_system_process_wait_status_hpp
