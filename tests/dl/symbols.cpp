// Fixture of dl_symbolize: built as libsymbols.so, without libc, with the test's flags (-g in
// debug builds); functions to find by address, and a call back into the host from inside it.

extern "C" auto host_capture(void) -> void;

namespace plugin {

	[[gnu::noinline]] auto deep(const int value) -> int {

		asm("");
		return value * 3 + 1;
	}

} // namespace plugin

extern "C" [[gnu::noinline]] auto plugin_entry(const int value) -> int {
	return plugin::deep(value) + 1;
}

extern "C" [[gnu::noinline]] auto plugin_call_back(void) -> int {

	host_capture();
	asm("");
	return 1;
}
