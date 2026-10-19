// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/io/memory_reader.hpp"
#include "5-collections/io/read_line.hpp"
#include "5-collections/io/read_to_end.hpp"

static_assert(ca::io::is_buffered_reader<ca::io::memory_reader>);

auto main(void) -> int {
	ca::io::memory_reader r{"one\ntwo\r\n\nlast"};
	auto first = r.read_until('\n');
	if (!first || *first != ca::sv::str{"one\n"})
		return 1;
	ca::dynamic_array<char> line;
	auto got = ca::io::read_line(r, line);
	if (!got || !*got || ca::sv::str{line.data(), line.size()} != ca::sv::str{"two\r"})
		return 2;
	got = ca::io::read_line(r, line);
	if (!got || !*got || line.size() != 0U)
		return 3;
	char two[2];
	auto part = r.read(two);
	if (!part || *part != 2U || two[0] != 'l')
		return 4;
	ca::dynamic_array<char> rest;
	auto all = ca::io::read_to_end(r, rest);
	if (!all || *all != 2U || ca::sv::str{rest.data(), rest.size()} != ca::sv::str{"st"})
		return 5;
	got = ca::io::read_line(r, line);
	auto end = r.read_until('\n');
	return got && !*got && end && end->empty() && r.at_end() && line.cleanup() && rest.cleanup() ? 0 : 6;
}
