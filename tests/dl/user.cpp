// Fixture of dl_library: built as libuser.so, needing libbase.so through $ORIGIN, without libc.
// GNU hash table, packed relative relocations, calls and data across both objects, a weak
// undefined symbol.

extern "C" {

	extern int base_counter;

	auto base_value(void) -> int;
	auto base_fast(void) -> int;
	auto host_note(int value) -> void;
	__attribute__((weak)) auto host_absent(void) -> void;

	const char* user_names[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight"};

	auto user_name(const int index) -> const char* {
		return user_names[index];
	}

	auto user_sum(const int value) -> int {

		host_note(10 + value);
		return value + base_value() + base_fast() + base_counter + (host_absent == nullptr ? 1000 : 0);
	}

}
