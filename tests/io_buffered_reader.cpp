// expect: exit 0
#include "3-system/file/read.hpp"
#include "3-system/file/write.hpp"
#include "3-system/io/pipe.hpp"
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/io/read_to_end.hpp"
#include "6-resources/fs/descriptor.hpp"
#include "6-resources/io/buffered_reader.hpp"

static_assert(ca::io::is_buffered_reader<ca::io::buffered_reader<>>);

auto main(void) -> int {
	auto ends = ca::sys::pipe(0);
	if (!ends)
		return 1;
	auto write_end = ca::fs::descriptor::adopt(ends->write_end());
	if (!ca::sys::write(write_end.native(), ca::sv::str{"first\nsecond line\n"}))
		return 2;
	auto borrowed = ca::io::buffered_reader<>::over(ends->read_end(), 8U);
	if (!borrowed || borrowed->owns_descriptor() || borrowed->capacity() != 8U)
		return 3;
	auto first = borrowed->read_until('\n');
	if (!first || *first != ca::sv::str{"first\n"} || borrowed->available() != 2U)
		return 4;
	if (borrowed->resize(1U) || !borrowed->resize(32U) || borrowed->available() != 2U)
		return 5;
	auto second = borrowed->read_until('\n');
	if (!second || *second != ca::sv::str{"second line\n"} || !borrowed->destroy() || borrowed->native() != -1)
		return 6;

	auto made = ca::arena::make();
	if (!made)
		return 7;
	ca::arena arena = ca::move(*made);
	{
		auto owner = ca::make_in<ca::io::buffered_reader<>>(arena, ca::fs::descriptor::adopt(ends->read_end()), 4U);
		if (!owner || !owner->owns_descriptor() || !ca::sys::write(write_end.native(), ca::sv::str{"abcdefghij"}) || !write_end.destroy())
			return 8;
		ca::dynamic_array<char> all;
		auto count = ca::io::read_to_end(*owner, all);
		if (!count || *count != 10U || ca::sv::str{all.data(), all.size()} != ca::sv::str{"abcdefghij"} || !all.destroy())
			return 9;
		if (!owner->destroy())
			return 10;
	}
	return arena.destroy() ? 0 : 11;
}
