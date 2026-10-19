// expect: exit 0
#include "3-system/file/read.hpp"
#include "3-system/file/write.hpp"
#include "3-system/io/dup2.hpp"
#include "3-system/io/pipe.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/io/read_line.hpp"
#include "6-resources/io/in.hpp"
#include "6-resources/print/print.hpp"

auto main(void) -> int {
	auto input = ca::sys::pipe(0);
	auto output = ca::sys::pipe(0);
	if (!input || !output || !ca::sys::dup2(input->read_end(), 0) || !ca::sys::dup2(output->write_end(), 1))
		return 1;
	if (!ca::sys::write(input->write_end(), ca::sv::str{"Ada\n"}))
		return 2;
	if (!ca::print("name? "))
		return 5;
	ca::dynamic_array<char> name;
	auto got = ca::io::read_line(ca::io::in(), name);
	if (!got || !*got || ca::sv::str{name.data(), name.size()} != ca::sv::str{"Ada"})
		return 3;
	char prompt[16];
	auto shown = ca::sys::read(output->read_end(), prompt);
	return shown && ca::sv::str{prompt, *shown} == ca::sv::str{"name? "} && name.cleanup() ? 0 : 4;
}
