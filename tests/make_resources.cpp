// expect: exit 0
// link: both
#include "3-system/error/errc.hpp"
#include "3-system/file/close.hpp"
#include "3-system/io/pipe.hpp"
#include "3-system/process/spawn_actions.hpp"
#include "4-lifecycle/make.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "5-collections/array/slice.hpp"
#include "5-collections/fs/path.hpp"
#include "5-collections/io/ring_buffer.hpp"
#include "5-collections/map/set.hpp"
#include "5-collections/string/str.hpp"
#include "6-resources/fs/descriptor.hpp"
#include "6-resources/io/buffered_reader.hpp"
#include "6-resources/io/buffered_writer.hpp"

auto main(void) -> int {
	auto file = ca::make<ca::fs::descriptor>("/dev/null", ca::fs::writable);
	if (!file || !file->is_open())
		return 1;
	auto missing = ca::make<ca::fs::descriptor>("/nonexistent/file");
	if (missing || missing.error() != ca::errc::no_entry)
		return 2;
	auto text = ca::make<ca::str>("a string long enough to live on the heap");
	auto path = ca::make<ca::fs::path>("a//b/");
	auto values = ca::make<ca::slice<int>>(3U, 7);
	const int raw[] = {2, 1, 2};
	auto keys = ca::make<ca::set<int>>(ca::borrowed_slice<const int>{raw});
	auto arena = ca::make<ca::arena>();
	auto ring = ca::make<ca::io::ring_buffer<>>(16U);
	auto actions = ca::make<ca::sys::spawn_actions>();
	if (!text || !path || path->view() != ca::sv::str{"a/b"} || !values || values->count(7) != 3U || !keys || keys->size() != 2U)
		return 3;
	if (!arena || !arena->holds() || !ring || ring->capacity() != 16U || !actions || !actions->is_live())
		return 4;

	auto ends = ca::sys::pipe(0);
	if (!ends)
		return 5;
	auto reader = ca::make<ca::io::buffered_reader<>>(ends->read_end(), 8U);
	auto writer = ca::make<ca::io::buffered_writer<>>(ca::fs::descriptor::adopt(ends->write_end()), ca::io::buffering::full, 8U);
	if (!reader || reader->owns_descriptor() || !writer || !writer->owns_descriptor())
		return 6;
	return file->cleanup() && text->cleanup() && path->cleanup() && values->cleanup() && keys->cleanup() && arena->cleanup()
		&& ring->cleanup() && actions->cleanup() && writer->cleanup() && reader->cleanup() && ca::sys::close(ends->read_end()) ? 0 : 7;
}
