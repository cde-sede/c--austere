// Fixture of dl_tls: built as libtlsbase.so, without libc. Thread-local storage reached through
// __tls_get_addr: an initialized exported variable (general dynamic), a static one (local
// dynamic), and zero-filled ones.

extern "C" {

	__thread int tls_counter = 5;

	auto tls_bump(void) -> int {
		return ++tls_counter;
	}

	auto tls_counter_address(void) -> int* {
		return &tls_counter;
	}

}

static __thread int hidden = 1;
static __thread char zeros[100];

extern "C" auto tls_hidden(void) -> int {
	return ++hidden;
}

extern "C" auto tls_zeros(void) -> int {

	int sum = 0;
	for (const char c : zeros)
		sum += c;
	zeros[99] = 1;
	return sum;
}
