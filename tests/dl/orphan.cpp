// Fixture of dl_library: built as liborphan.so against libghost.so, whose DT_SONAME
// (libca_ghost.so) names no file, so its DT_NEEDED cannot be found.

extern "C" auto broken(void) -> void;

extern "C" auto orphan(void) -> void {
	broken();
}
