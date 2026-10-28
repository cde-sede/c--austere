#ifndef ca_system_io_terminal_settings_hpp
#define ca_system_io_terminal_settings_hpp

#include "1-primitives/config/platform.hpp"
#include "1-primitives/debug/trap.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "3-system/io/terminal_flags.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TERMINAL SETTINGS ----------------------------------------------

		namespace impl {

			struct terminal_layout;

			[[noreturn, gnu::cold, gnu::noinline]] inline auto terminal_character_out_of_range(void) noexcept -> void {
				ca::trap();
			}

		} // namespace impl


		/* @DOC A terminal's settings, struct termios without <termios.h>: the four flag words
		   (sys::terminal_input, terminal_output, terminal_control, terminal_local) and the control
		   characters (slots in sys::terminal_character; character and set_character trap past
		   count). On Linux it has the kernel's layout and goes to ioctl uncopied. */
		class terminal_settings final {


			private:

				// -- friends ------------------------------------------------

				friend struct ca::sys::impl::terminal_layout;


				// -- private types ------------------------------------------

				using self = ca::sys::terminal_settings;


				// -- private members ----------------------------------------

				ca::u32 _input;
				ca::u32 _output;
				ca::u32 _control;
				ca::u32 _local;
#if ca_platform(linux)
				ca::u8 _line;
				ca::u8 _characters[ca::sys::terminal_character::count];
#else
				ca::u8 _characters[ca::sys::terminal_character::count];
				ca::u32 _input_speed;
				ca::u32 _output_speed;
#endif


			public:

				// -- public lifecycle ---------------------------------------

#if ca_platform(linux)
				constexpr terminal_settings(void) noexcept
				: _input{0U}, _output{0U}, _control{0U}, _local{0U}, _line{0U}, _characters{} {
				}
#else
				constexpr terminal_settings(void) noexcept
				: _input{0U}, _output{0U}, _control{0U}, _local{0U}, _characters{}, _input_speed{0U}, _output_speed{0U} {
				}
#endif


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto input(void) const noexcept -> ca::u32 {
					return _input;
				}

				[[nodiscard]] constexpr auto output(void) const noexcept -> ca::u32 {
					return _output;
				}

				[[nodiscard]] constexpr auto control(void) const noexcept -> ca::u32 {
					return _control;
				}

				[[nodiscard]] constexpr auto local(void) const noexcept -> ca::u32 {
					return _local;
				}

				[[nodiscard]] constexpr auto character(const ca::usz slot) const noexcept -> ca::u8 {

					if (slot >= ca::sys::terminal_character::count) [[unlikely]]
						ca::sys::impl::terminal_character_out_of_range();
					return _characters[slot];
				}

				[[nodiscard]] constexpr auto operator==(const self&) const noexcept -> bool = default;


				// -- modifiers ----------------------------------------------

				constexpr auto set_input(const ca::u32 flags) noexcept -> self& {

					_input = flags;
					return *this;
				}

				constexpr auto set_output(const ca::u32 flags) noexcept -> self& {

					_output = flags;
					return *this;
				}

				constexpr auto set_control(const ca::u32 flags) noexcept -> self& {

					_control = flags;
					return *this;
				}

				constexpr auto set_local(const ca::u32 flags) noexcept -> self& {

					_local = flags;
					return *this;
				}

				constexpr auto set_character(const ca::usz slot, const ca::u8 value) noexcept -> self& {

					if (slot >= ca::sys::terminal_character::count) [[unlikely]]
						ca::sys::impl::terminal_character_out_of_range();
					_characters[slot] = value;
					return *this;
				}


				// -- methods ------------------------------------------------

				/* @DOC These settings in raw mode, as cfmakeraw makes them: bytes pass through
				   unprocessed and unechoed, one at a time (minimum 1, time 0), with no signal keys
				   and no flow control. */
				[[nodiscard]] constexpr auto raw(void) const noexcept -> self {

					self made = *this;
					made._input &= ~(ca::sys::terminal_input::ignore_break | ca::sys::terminal_input::break_interrupts
						| ca::sys::terminal_input::mark_parity | ca::sys::terminal_input::strip
						| ca::sys::terminal_input::newline_to_return | ca::sys::terminal_input::ignore_return
						| ca::sys::terminal_input::return_to_newline | ca::sys::terminal_input::output_flow);
					made._output &= ~ca::sys::terminal_output::process;
					made._local &= ~(ca::sys::terminal_local::echo | ca::sys::terminal_local::echo_newline
						| ca::sys::terminal_local::canonical | ca::sys::terminal_local::signals
						| ca::sys::terminal_local::extended);
					made._control &= ~(ca::sys::terminal_control::size | ca::sys::terminal_control::parity);
					made._control |= ca::sys::terminal_control::size_8;
					made._characters[ca::sys::terminal_character::minimum] = 1U;
					   made._characters[ca::sys::terminal_character::time] = 0U;
					return made;
				}

		}; // class terminal_settings

	} // namespace sys

} // namespace ca

#endif // ca_system_io_terminal_settings_hpp
