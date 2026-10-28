#ifndef ca_system_process_signal_info_hpp
#define ca_system_process_signal_info_hpp

#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SIGNAL CODES ---------------------------------------------------

		/* @DOC How a signal was sent, as signal_info::code reports it: user by kill or raise, queue by
		   sigqueue. Values are checked against <signal.h> when the library builds. */
		namespace signal_codes {

#if ca_platform(linux)
			inline constexpr int user  = 0;
			inline constexpr int queue = -1;
#else
			inline constexpr int user  = 0x10001;
			inline constexpr int queue = 0x10002;
#endif

		} // namespace signal_codes


		// -- SIGNAL INFO ----------------------------------------------------

		/* @DOC What sigwaitinfo and sigtimedwait take with a signal: its number, how it was sent
		   (signal_codes), and for user and queue its sender's pid and real uid, for queue the value
		   sigqueue carried. */
		class signal_info final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::signal_info;


				// -- private members ----------------------------------------

				int _signal;
				int _code;
				ca::i32 _sender;
				ca::u32 _user;
				ca::i64 _value;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr signal_info(const int signal, const int code, const ca::i32 sender, const ca::u32 user, const ca::i64 value) noexcept
				: _signal{signal}, _code{code}, _sender{sender}, _user{user}, _value{value} {
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto signal(void) const noexcept -> int {
					return _signal;
				}

				[[nodiscard]] constexpr auto code(void) const noexcept -> int {
					return _code;
				}

				/* @DOC Whether sigqueue sent it, so value is meaningful. */
				[[nodiscard]] constexpr auto queued(void) const noexcept -> bool {
					return _code == ca::sys::signal_codes::queue;
				}

				[[nodiscard]] constexpr auto sender(void) const noexcept -> ca::i32 {
					return _sender;
				}

				[[nodiscard]] constexpr auto user(void) const noexcept -> ca::u32 {
					return _user;
				}

				[[nodiscard]] constexpr auto value(void) const noexcept -> ca::i64 {
					return _value;
				}

		}; // class signal_info

	} // namespace sys

} // namespace ca

#endif // ca_system_process_signal_info_hpp
