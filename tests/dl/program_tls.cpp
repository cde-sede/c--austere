// Fixture of dl_program_symbols: built as libprogram_tls.so, without libc; a thread-local
// variable it leaves undefined, which the test program defines and exports.

extern "C" {

	extern thread_local int program_tls;

	auto plugin_tls(void) -> int {
		return program_tls;
	}

}
