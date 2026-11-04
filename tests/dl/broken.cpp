// Fixture of dl_library: built as libbroken.so, needing a symbol nobody defines.

extern "C" auto host_missing(void) -> void;

extern "C" auto broken(void) -> void {
	host_missing();
}
