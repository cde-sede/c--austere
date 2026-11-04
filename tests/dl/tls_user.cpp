// Fixture of dl_tls: built as libtlsuser.so, needing libtlsbase.so; reads its thread-local
// variable from another module.

extern "C" {

	extern __thread int tls_counter;

	auto tls_user_read(void) -> int {
		return tls_counter * 10;
	}

}
