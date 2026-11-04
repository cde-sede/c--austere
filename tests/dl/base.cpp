// Fixture of dl_library: built as libbase.so, without libc. SysV hash table only, function
// pointers in data, an exported and a local indirect function, an initializer and a
// finalizer calling back into the host.

extern "C" void host_note(int value);

static auto twice(const int value) -> int {
	return 2 * value;
}

static auto thrice(const int value) -> int {
	return 3 * value;
}

static auto fast(void) -> int {
	return 7;
}

extern "C" {

	int base_counter = 0;

	int (*base_table[2])(int) = {twice, thrice};

	auto resolve_fast(void) -> int (*)(void) {
		return fast;
	}

	__attribute__((ifunc("resolve_fast"))) auto base_fast(void) -> int;

	auto base_value(void) -> int {
		return 40 + base_table[0](1) + base_table[1](0);
	}

}

__attribute__((ifunc("resolve_fast"))) static auto local_fast(void) -> int;

extern "C" auto base_local_fast(void) -> int {
	return local_fast() + 1;
}

__attribute__((constructor)) static auto start(void) -> void {
	base_counter += 100;
	host_note(1);
}

__attribute__((destructor)) static auto stop(void) -> void {
	host_note(2);
}
