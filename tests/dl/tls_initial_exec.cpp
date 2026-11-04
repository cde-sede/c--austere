// Fixture of dl_tls: built as libtlsie.so with -ftls-model=initial-exec, which needs static TLS
// the loader cannot give.

extern "C" {

	__thread int tls_fixed = 3;

	auto tls_fixed_read(void) -> int {
		return tls_fixed;
	}

}
