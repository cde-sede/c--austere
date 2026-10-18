// expect: exit 0
#include "3-system/error/errc.hpp"
#include "6-resources/io/writer.hpp"
#include "6-resources/print/print.hpp"

auto main(void) -> int {
	char buffer[16];
	ca::io::writer closed{99, ca::io::buffering::none, buffer};
	auto printed = ca::print(closed, "{}", 1);
	if (printed || printed.error() != ca::errc::bad_descriptor)
		return 1;
	if (closed.error() != ca::errc::bad_descriptor || closed.pending() != 1U)
		return 2;
	closed.clear_error();
	if (!closed.error().ok())
		return 3;
	auto retried = closed.flush();
	if (retried || retried.error() != ca::errc::bad_descriptor || closed.pending() != 1U || !closed.error().ok())
		return 4;
	closed.discard();
	return closed.pending() == 0U && closed.flush() ? 0 : 5;
}
