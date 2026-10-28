#ifndef ca_system_process_wait_status_hpp
#define ca_system_process_wait_status_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- WAIT STATUS ----------------------------------------------------

		/* @DOC How a child process ended: which child, and an exit code or the signal that killed it. */
		class wait_status final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::wait_status;


				// -- private members ----------------------------------------

				ca::i32 _child;
				bool _exited;
				int _value;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr wait_status(const ca::i32 child, const bool exited, const int value) noexcept
				: _child{child}, _exited{exited}, _value{value} {
				}


				// -- accessors ----------------------------------------------

				/* @DOC Id of the child reaped: the one a waitpid for -1 or a process group found. */
				[[nodiscard]] constexpr auto child(void) const noexcept -> ca::i32 {
					return _child;
				}

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
