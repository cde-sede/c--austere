// Fixture of dl_program_symbols and dl_program_hidden: built as libprogram.so and
// libprogram_host.so, without libc; a function and a variable it leaves undefined, which the
// test program defines and exports.

extern "C" {

	extern int program_counter;

	auto program_add(int value) -> int;
	auto program_shadowed(void) -> int;

	auto plugin_run(const int value) -> int {
		return program_add(value) + program_counter;
	}

	auto plugin_bump(void) -> void {
		++program_counter;
	}

	auto plugin_shadowed(void) -> int {
		return program_shadowed();
	}

}
