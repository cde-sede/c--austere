// expect: trap
#include "1-primitives/utility/move.hpp"
#include "3-system/io/pipe.hpp"
#include "5-collections/format/format_to.hpp"
#include "6-resources/io/buffered_writer.hpp"

auto main(void) -> int {
	auto ends = ca::sys::pipe(0);
	if (!ends)
		return 1;
	auto writer = ca::io::buffered_writer<>::over(ends->write_end(), ca::io::buffering::full, 8U);
	auto other = ca::io::buffered_writer<>::over(ends->write_end(), ca::io::buffering::full, 8U);
	if (!writer || !other || !ca::format_to(*writer, "{}", 1))
		return 2;
	*writer = ca::move(*other);
	return 0;
}
