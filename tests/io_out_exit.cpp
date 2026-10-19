// expect: exit 0
// link: both
#include "3-system/file/read.hpp"
#include "3-system/io/dup2.hpp"
#include "3-system/io/pipe.hpp"
#include "3-system/process/exit.hpp"
#include "3-system/process/fork.hpp"
#include "3-system/process/waitpid.hpp"
#include "6-resources/io/err.hpp"
#include "6-resources/print/print.hpp"
#include "6-resources/print/printf.hpp"

auto main(void) -> int {
	auto ends = ca::sys::pipe(0);
	if (!ends)
		return 1;
	auto child = ca::sys::fork();
	if (!child)
		return 2;
	if (*child == 0) {
		if (!ca::sys::dup2(ends->write_end(), 1) || !ca::sys::dup2(ends->write_end(), 2))
			ca::sys::exit(3);
		if (!ca::print("buffered {} ", 1) || !ca::print(ca::io::err(), "err ") || !ca::printf("%s\n", "at exit"))
			ca::sys::exit(6);
		ca::sys::exit(0);
	}
	auto ended = ca::sys::waitpid(*child);
	if (!ended || !ended->succeeded())
		return 4;
	char got[64];
	auto size = ca::sys::read(ends->read_end(), got);
	return size && ca::sv::str{got, *size} == ca::sv::str{"err buffered 1 at exit\n"} ? 0 : 5;
}
