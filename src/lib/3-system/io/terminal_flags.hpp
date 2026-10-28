#ifndef ca_system_io_terminal_flags_hpp
#define ca_system_io_terminal_flags_hpp

#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TERMINAL FLAGS -------------------------------------------------

		/* @DOC termios input flags (c_iflag), without <termios.h>. These, and the other terminal
		   flags and character slots, are checked against it when the library builds. */
		namespace terminal_input {

#if ca_platform(linux)
			inline constexpr ca::u32 ignore_break      = 01;
			inline constexpr ca::u32 break_interrupts  = 02;
			inline constexpr ca::u32 ignore_parity     = 04;
			inline constexpr ca::u32 mark_parity       = 010;
			inline constexpr ca::u32 check_parity      = 020;
			inline constexpr ca::u32 strip             = 040;
			inline constexpr ca::u32 newline_to_return = 0100;
			inline constexpr ca::u32 ignore_return     = 0200;
			inline constexpr ca::u32 return_to_newline = 0400;
			inline constexpr ca::u32 output_flow       = 02000;
			inline constexpr ca::u32 input_flow        = 010000;
#else
			inline constexpr ca::u32 ignore_break      = 0x1;
			inline constexpr ca::u32 break_interrupts  = 0x2;
			inline constexpr ca::u32 ignore_parity     = 0x4;
			inline constexpr ca::u32 mark_parity       = 0x8;
			inline constexpr ca::u32 check_parity      = 0x10;
			inline constexpr ca::u32 strip             = 0x20;
			inline constexpr ca::u32 newline_to_return = 0x40;
			inline constexpr ca::u32 ignore_return     = 0x80;
			inline constexpr ca::u32 return_to_newline = 0x100;
			inline constexpr ca::u32 output_flow       = 0x200;
			inline constexpr ca::u32 input_flow        = 0x400;
#endif

		} // namespace terminal_input


		/* @DOC termios output flags (c_oflag). */
		namespace terminal_output {

#if ca_platform(linux)
			inline constexpr ca::u32 process           = 01;
			inline constexpr ca::u32 newline_to_crlf   = 04;
#else
			inline constexpr ca::u32 process           = 0x1;
			inline constexpr ca::u32 newline_to_crlf   = 0x2;
#endif

		} // namespace terminal_output


		/* @DOC termios control flags (c_cflag). size is the mask over size_5 to size_8. */
		namespace terminal_control {

#if ca_platform(linux)
			inline constexpr ca::u32 size              = 060;
			inline constexpr ca::u32 size_5            = 0;
			inline constexpr ca::u32 size_6            = 020;
			inline constexpr ca::u32 size_7            = 040;
			inline constexpr ca::u32 size_8            = 060;
			inline constexpr ca::u32 two_stop_bits     = 0100;
			inline constexpr ca::u32 receive           = 0200;
			inline constexpr ca::u32 parity            = 0400;
#else
			inline constexpr ca::u32 size              = 0x300;
			inline constexpr ca::u32 size_5            = 0;
			inline constexpr ca::u32 size_6            = 0x100;
			inline constexpr ca::u32 size_7            = 0x200;
			inline constexpr ca::u32 size_8            = 0x300;
			inline constexpr ca::u32 two_stop_bits     = 0x400;
			inline constexpr ca::u32 receive           = 0x800;
			inline constexpr ca::u32 parity            = 0x1000;
#endif

		} // namespace terminal_control


		/* @DOC termios local flags (c_lflag). */
		namespace terminal_local {

#if ca_platform(linux)
			inline constexpr ca::u32 signals           = 01;
			inline constexpr ca::u32 canonical         = 02;
			inline constexpr ca::u32 echo              = 010;
			inline constexpr ca::u32 echo_erase        = 020;
			inline constexpr ca::u32 echo_kill         = 040;
			inline constexpr ca::u32 echo_newline      = 0100;
			inline constexpr ca::u32 no_flush          = 0200;
			inline constexpr ca::u32 stop_background   = 0400;
			inline constexpr ca::u32 extended          = 0100000;
#else
			inline constexpr ca::u32 signals           = 0x80;
			inline constexpr ca::u32 canonical         = 0x100;
			inline constexpr ca::u32 echo              = 0x8;
			inline constexpr ca::u32 echo_erase        = 0x2;
			inline constexpr ca::u32 echo_kill         = 0x4;
			inline constexpr ca::u32 echo_newline      = 0x10;
			inline constexpr ca::u32 no_flush          = 0x80000000;
			inline constexpr ca::u32 stop_background   = 0x400000;
			inline constexpr ca::u32 extended          = 0x400;
#endif

		} // namespace terminal_local


		/* @DOC Slots of the termios control characters (c_cc). minimum and time drive reads outside
		   canonical mode: a read returns once minimum bytes arrived, or time tenths of a second
		   after the last one. count is the number of slots. */
		namespace terminal_character {

#if ca_platform(linux)
			inline constexpr ca::usz interrupt         = 0;
			inline constexpr ca::usz quit              = 1;
			inline constexpr ca::usz erase             = 2;
			inline constexpr ca::usz kill              = 3;
			inline constexpr ca::usz end_of_file       = 4;
			inline constexpr ca::usz time              = 5;
			inline constexpr ca::usz minimum           = 6;
			inline constexpr ca::usz start             = 8;
			inline constexpr ca::usz stop              = 9;
			inline constexpr ca::usz suspend           = 10;
			inline constexpr ca::usz count             = 19;
#else
			inline constexpr ca::usz end_of_file       = 0;
			inline constexpr ca::usz erase             = 3;
			inline constexpr ca::usz kill              = 5;
			inline constexpr ca::usz interrupt         = 8;
			inline constexpr ca::usz quit              = 9;
			inline constexpr ca::usz suspend           = 10;
			inline constexpr ca::usz start             = 12;
			inline constexpr ca::usz stop              = 13;
			inline constexpr ca::usz minimum           = 16;
			inline constexpr ca::usz time              = 17;
			inline constexpr ca::usz count             = 20;
#endif

		} // namespace terminal_character

	} // namespace sys

} // namespace ca

#endif // ca_system_io_terminal_flags_hpp
