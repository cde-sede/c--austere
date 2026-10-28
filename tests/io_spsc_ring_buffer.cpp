// expect: exit 0
// link: libc
#include "2-vocabulary/io/is_buffered_reader.hpp"
#include "2-vocabulary/io/is_stream.hpp"
#include "2-vocabulary/io/is_windowed_writer.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"
#include "5-collections/format/format_to.hpp"
#include "5-collections/io/spsc_ring_buffer.hpp"

#include <pthread.h>

static_assert(ca::io::is_stream<ca::io::spsc_ring_buffer<>>);
static_assert(ca::io::is_buffered_reader<ca::io::spsc_ring_buffer<>>);
static_assert(ca::io::is_windowed_writer<ca::io::spsc_ring_buffer<>>);

static constexpr int lines = 100000;

static ca::io::spsc_ring_buffer<>* shared = nullptr;

static auto produce(void*) -> void* {

	for (int i = 0; i < lines; ++i) {
		while (!ca::format_to(*shared, "{} {}\n", i, i * 3))
			static_cast<void>(shared->flush());
	}
	return nullptr;
}

static auto parse(const char* at, const char* const end, long& value) -> const char* {

	value = 0;
	while (at != end && *at >= '0' && *at <= '9')
		value = value * 10 + (*at++ - '0');
	return at;
}

auto main(void) -> int {
	auto made = ca::io::spsc_ring_buffer<>::make(10U);
	if (!made || made->capacity() != 16U || made->size() != 0U)
		return 1;
	auto ring = ca::move(*made);
	char out[16];
	auto nothing = ring.read(out);
	if (!nothing || *nothing != 0U)
		return 2;
	if (!ring.write(ca::sv::str{"hello, world"}) || ring.size() != 12U || ring.space() != 4U)
		return 3;
	auto refused = ring.write(ca::sv::str{"12345"});
	if (refused || refused.error() != ca::errc::no_space || ring.error() != ca::errc::no_space)
		return 4;
	auto sticky = ring.write(ca::sv::str{"1"});
	if (sticky || ring.flush() || !ring.error().ok())
		return 5;
	auto got = ring.read(ca::span{out, 7U});
	if (!got || *got != 7U || ca::sv::str{out, 7U} != ca::sv::str{"hello, "})
		return 6;
	if (!ring.write(ca::sv::str{"wrapped"}))
		return 7;
	auto front = ring.fill();
	if (!front || *front != ca::sv::str{"worldwrap"})
		return 8;
	ring.consume(front->size());
	auto rest = ring.fill();
	if (!rest || *rest != ca::sv::str{"ped"})
		return 9;
	ring.consume(100U);
	auto none = ring.fill();
	if (ring.size() != 0U || !none || !none->empty())
		return 10;
	if (!ca::format_to(ring, "{}-{}", 12345, 678) || ring.size() != 9U)
		return 11;
	auto split = ring.read(out);
	if (!split || ca::sv::str{out, *split} != ca::sv::str{"12345-678"})
		return 12;
	auto huge = ca::format_to(ring, "{}", ca::sv::str{"0123456789abcdefg"});
	if (huge || ring.size() != 0U || ring.flush())
		return 13;
	if (!ring.cleanup() || ring.capacity() != 0U || !ring.cleanup())
		return 14;

	auto pipe = ca::io::spsc_ring_buffer<>::make(256U);
	if (!pipe)
		return 15;
	shared = &*pipe;
	pthread_t producer;
	if (pthread_create(&producer, nullptr, produce, nullptr) != 0)
		return 16;
	char line[64];
	ca::usz held = 0U;
	int expected = 0;
	while (expected < lines) {
		auto read = shared->read(ca::span{line + held, sizeof line - held});
		if (!read)
			return 17;
		held += *read;
		const char* at = line;
		const char* const end = line + held;
		for (;;) {
			const char* newline = at;
			while (newline != end && *newline != '\n')
				++newline;
			if (newline == end)
				break;
			long first = 0;
			long second = 0;
			const char* cursor = parse(at, newline, first);
			if (cursor == newline || *cursor != ' ' || parse(cursor + 1, newline, second) != newline)
				return 18;
			if (first != expected || second != expected * 3L)
				return 19;
			++expected;
			at = newline + 1;
		}
		held = static_cast<ca::usz>(end - at);
		for (ca::usz i = 0U; i < held; ++i)
			line[i] = at[i];
	}
	pthread_join(producer, nullptr);
	return held == 0U && shared->size() == 0U && pipe->cleanup() ? 0 : 20;
}
