// expect: exit 0
#include "3-system/file/read.hpp"
#include "3-system/fs/options.hpp"
#include "3-system/io/pipe.hpp"
#include "5-collections/format/format_to.hpp"
#include "6-resources/fs/descriptor.hpp"
#include "6-resources/io/writer.hpp"

auto main(void) -> int {
	auto ends = ca::sys::pipe(ca::fs::nonblock_t::OPEN_VALUE);
	if (!ends)
		return 1;
	auto out = ca::fs::descriptor::adopt(ends->write_end());
	char small[4];
	ca::io::writer s{out, ca::io::buffering::full, small};
	if (!ca::format_to(s, "ab"))
		return 6;
	char large[64];
	if (!s.set_buffer(large) || s.capacity() != 64U || s.buffer().data() != large || s.pending() != 0U)
		return 2;
	auto queued = ca::format_to(s, "{}", "cdefgh");
	if (!queued || *queued != 6U)
		return 7;
	char got[16];
	auto first = ca::sys::read(ends->read_end(), got);
	if (!first || ca::sv::str{got, *first} != ca::sv::str{"ab"} || s.pending() != 6U)
		return 3;
	if (!s.set_buffer(ca::span<char>{}) || s.capacity() != 0U)
		return 4;
	auto second = ca::sys::read(ends->read_end(), got);
	return second && ca::sv::str{got, *second} == ca::sv::str{"cdefgh"} && out.cleanup() ? 0 : 5;
}
