// expect: exit 0
// link: both
#include "1-primitives/utility/move.hpp"
#include "3-system/file/close.hpp"
#include "3-system/io/pipe.hpp"
#include "5-collections/format/format_to.hpp"
#include "5-collections/io/ring_buffer.hpp"
#include "6-resources/io/buffered_reader.hpp"
#include "6-resources/io/buffered_writer.hpp"

auto main(void) -> int {
	auto ends = ca::sys::pipe(0);
	if (!ends)
		return 1;
	auto reader = ca::io::buffered_reader<>::over(ends->read_end(), 8U);
	auto bigger = ca::io::buffered_reader<>::over(ends->read_end(), 64U);
	if (!reader || !bigger)
		return 2;
	*reader = ca::move(*bigger);
	if (reader->capacity() != 64U || bigger->capacity() != 0U || reader->native() != ends->read_end())
		return 3;

	auto writer = ca::io::buffered_writer<>::over(ends->write_end(), ca::io::buffering::full, 8U);
	auto other = ca::io::buffered_writer<>::over(ends->write_end(), ca::io::buffering::full, 32U);
	if (!writer || !other || !ca::format_to(*other, "{}", 42) || !other->flush())
		return 4;
	*writer = ca::move(*other);
	if (writer->capacity() != 32U || other->capacity() != 0U)
		return 5;

	auto ring = ca::io::ring_buffer<>::make(16U);
	auto wider = ca::io::ring_buffer<>::make(64U);
	if (!ring || !wider || !ring->write(ca::sv::str{"lost"}))
		return 6;
	*ring = ca::move(*wider);
	if (ring->capacity() != 64U || !ring->empty() || wider->capacity() != 0U)
		return 7;
	return reader->cleanup() && writer->cleanup() && ring->cleanup() && ca::sys::close(ends->read_end()) && ca::sys::close(ends->write_end()) ? 0 : 8;
}
