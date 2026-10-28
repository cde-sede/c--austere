#include "1-primitives/config/platform.hpp"
#include "3-system/io/impl/terminal_layout.hpp"

#if ca_platform(linux)
#include <asm/termbits.h>
#else
#include <termios.h>
#endif


static_assert(ca::sys::terminal_input::ignore_break == IGNBRK);
static_assert(ca::sys::terminal_input::break_interrupts == BRKINT);
static_assert(ca::sys::terminal_input::ignore_parity == IGNPAR);
static_assert(ca::sys::terminal_input::mark_parity == PARMRK);
static_assert(ca::sys::terminal_input::check_parity == INPCK);
static_assert(ca::sys::terminal_input::strip == ISTRIP);
static_assert(ca::sys::terminal_input::newline_to_return == INLCR);
static_assert(ca::sys::terminal_input::ignore_return == IGNCR);
static_assert(ca::sys::terminal_input::return_to_newline == ICRNL);
static_assert(ca::sys::terminal_input::output_flow == IXON);
static_assert(ca::sys::terminal_input::input_flow == IXOFF);
static_assert(ca::sys::terminal_output::process == OPOST);
static_assert(ca::sys::terminal_output::newline_to_crlf == ONLCR);
static_assert(ca::sys::terminal_control::size == CSIZE);
static_assert(ca::sys::terminal_control::size_5 == CS5);
static_assert(ca::sys::terminal_control::size_6 == CS6);
static_assert(ca::sys::terminal_control::size_7 == CS7);
static_assert(ca::sys::terminal_control::size_8 == CS8);
static_assert(ca::sys::terminal_control::two_stop_bits == CSTOPB);
static_assert(ca::sys::terminal_control::receive == CREAD);
static_assert(ca::sys::terminal_control::parity == PARENB);
static_assert(ca::sys::terminal_local::signals == ISIG);
static_assert(ca::sys::terminal_local::canonical == ICANON);
static_assert(ca::sys::terminal_local::echo == ECHO);
static_assert(ca::sys::terminal_local::echo_erase == ECHOE);
static_assert(ca::sys::terminal_local::echo_kill == ECHOK);
static_assert(ca::sys::terminal_local::echo_newline == ECHONL);
static_assert(ca::sys::terminal_local::no_flush == NOFLSH);
static_assert(ca::sys::terminal_local::stop_background == TOSTOP);
static_assert(ca::sys::terminal_local::extended == IEXTEN);
static_assert(ca::sys::terminal_character::interrupt == VINTR);
static_assert(ca::sys::terminal_character::quit == VQUIT);
static_assert(ca::sys::terminal_character::erase == VERASE);
static_assert(ca::sys::terminal_character::kill == VKILL);
static_assert(ca::sys::terminal_character::end_of_file == VEOF);
static_assert(ca::sys::terminal_character::time == VTIME);
static_assert(ca::sys::terminal_character::minimum == VMIN);
static_assert(ca::sys::terminal_character::start == VSTART);
static_assert(ca::sys::terminal_character::stop == VSTOP);
static_assert(ca::sys::terminal_character::suspend == VSUSP);
static_assert(ca::sys::terminal_character::count == NCCS);

auto ca::sys::impl::terminal_layout::to_native(const ca::sys::terminal_settings& settings, void* const native) noexcept -> void {

#if ca_platform(linux)
	static_assert(sizeof(ca::sys::terminal_settings) == sizeof(::termios) && alignof(ca::sys::terminal_settings) == alignof(::termios));
	static_assert(__builtin_offsetof(ca::sys::terminal_settings, _input) == __builtin_offsetof(::termios, c_iflag));
	static_assert(__builtin_offsetof(ca::sys::terminal_settings, _output) == __builtin_offsetof(::termios, c_oflag));
	static_assert(__builtin_offsetof(ca::sys::terminal_settings, _control) == __builtin_offsetof(::termios, c_cflag));
	static_assert(__builtin_offsetof(ca::sys::terminal_settings, _local) == __builtin_offsetof(::termios, c_lflag));
	static_assert(__builtin_offsetof(ca::sys::terminal_settings, _line) == __builtin_offsetof(::termios, c_line));
	static_assert(__builtin_offsetof(ca::sys::terminal_settings, _characters) == __builtin_offsetof(::termios, c_cc));
#endif
	auto& to = *static_cast<::termios*>(native);
	to.c_iflag = settings._input;
	to.c_oflag = settings._output;
	to.c_cflag = settings._control;
	to.c_lflag = settings._local;
	for (ca::usz slot = 0U; slot != ca::sys::terminal_character::count; ++slot)
		to.c_cc[slot] = settings._characters[slot];
#if ca_platform(linux)
	to.c_line = settings._line;
#else
	to.c_ispeed = settings._input_speed;
	to.c_ospeed = settings._output_speed;
#endif
}

[[nodiscard]] auto ca::sys::impl::terminal_layout::from_native(const void* const native) noexcept -> ca::sys::terminal_settings {

	const auto& from = *static_cast<const ::termios*>(native);
	ca::sys::terminal_settings settings{};
	  settings._input = static_cast<ca::u32>(from.c_iflag);
	 settings._output = static_cast<ca::u32>(from.c_oflag);
	settings._control = static_cast<ca::u32>(from.c_cflag);
	  settings._local = static_cast<ca::u32>(from.c_lflag);
	for (ca::usz slot = 0U; slot != ca::sys::terminal_character::count; ++slot)
		settings._characters[slot] = static_cast<ca::u8>(from.c_cc[slot]);
#if ca_platform(linux)
	settings._line = static_cast<ca::u8>(from.c_line);
#else
	 settings._input_speed = static_cast<ca::u32>(from.c_ispeed);
	settings._output_speed = static_cast<ca::u32>(from.c_ospeed);
#endif
	return settings;
}
