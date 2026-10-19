// Benchmarks for ca::io::ring_buffer; -DUSE_STD runs the same cases on std::deque<char>, the
// std byte FIFO (insert at the back, copy and erase at the front), with std::format_to through
// a back_inserter for the formatting case.

#include "measure.hpp"

static const int ROUNDS = 1000000;
static const unsigned CAPACITY = 4096U;

static char message[256];

static auto size_of(const int i) -> unsigned {
	return 1U + (static_cast<unsigned>(i) * 2654435761U >> 24U);
}

#ifdef USE_STD
#include <algorithm>
#include <deque>
#include <format>
#include <iterator>

NI static auto fixed(void) -> void {
	std::deque<char> fifo;
	char out[64];
	size_t sum = 0;
	for (int i = 0; i < ROUNDS; ++i) {
		fifo.insert(fifo.end(), message, message + 64);
		std::copy_n(fifo.begin(), 64, out);
		fifo.erase(fifo.begin(), fifo.begin() + 64);
		sum += static_cast<unsigned char>(out[i & 63]);
	}
	sink = sum;
}

NI static auto mixed(void) -> void {
	std::deque<char> fifo;
	char out[256];
	size_t sum = 0;
	for (int i = 0; i < ROUNDS; ++i) {
		const unsigned size = size_of(i);
		if (fifo.size() + size <= CAPACITY)
			fifo.insert(fifo.end(), message, message + size);
		if (fifo.size() >= CAPACITY / 2U) {
			const size_t count = std::min<size_t>(fifo.size(), 200U);
			std::copy_n(fifo.begin(), count, out);
			fifo.erase(fifo.begin(), fifo.begin() + static_cast<long>(count));
			sum += count + static_cast<unsigned char>(out[0]);
		}
	}
	sink = sum;
}

NI static auto formatted(void) -> void {
	std::deque<char> fifo;
	char out[CAPACITY];
	size_t sum = 0;
	for (int i = 0; i < ROUNDS; ++i) {
		std::format_to(std::back_inserter(fifo), "{} {}\n", i, i * 3);
		if (fifo.size() >= CAPACITY - 64U) {
			const size_t count = fifo.size();
			std::copy_n(fifo.begin(), count, out);
			fifo.clear();
			sum += count + static_cast<unsigned char>(out[count - 1U]);
		}
	}
	sink = sum;
}
#else
#include "5-collections/format/format_to.hpp"
#include "5-collections/io/ring_buffer.hpp"

NI static auto fixed(void) -> void {
	auto ring = ca::io::ring_buffer<>::make(CAPACITY);
	char out[64];
	size_t sum = 0;
	for (int i = 0; i < ROUNDS; ++i) {
		static_cast<void>(ring->write(ca::span{message, 64U}));
		static_cast<void>(ring->read(out));
		sum += static_cast<unsigned char>(out[i & 63]);
	}
	static_cast<void>(ring->cleanup());
	sink = sum;
}

NI static auto mixed(void) -> void {
	auto ring = ca::io::ring_buffer<>::make(CAPACITY);
	char out[256];
	size_t sum = 0;
	for (int i = 0; i < ROUNDS; ++i) {
		const unsigned size = size_of(i);
		if (ring->space() >= size)
			static_cast<void>(ring->write(ca::span{message, size}));
		if (ring->size() >= CAPACITY / 2U) {
			auto got = ring->read(ca::span{out, 200U});
			sum += *got + static_cast<unsigned char>(out[0]);
		}
	}
	static_cast<void>(ring->cleanup());
	sink = sum;
}

NI static auto formatted(void) -> void {
	auto ring = ca::io::ring_buffer<>::make(CAPACITY);
	char out[CAPACITY];
	size_t sum = 0;
	for (int i = 0; i < ROUNDS; ++i) {
		static_cast<void>(ca::format_to(*ring, "{} {}\n", i, i * 3));
		if (ring->size() >= CAPACITY - 64U) {
			auto got = ring->read(out);
			sum += *got + static_cast<unsigned char>(out[*got - 1U]);
		}
	}
	static_cast<void>(ring->cleanup());
	sink = sum;
}
#endif


// -- MAIN -------------------------------------------------------------------

auto main(void) -> int {
	for (unsigned i = 0; i < sizeof message; ++i)
		message[i] = static_cast<char>('a' + i % 26U);
	if (!open_counters())
		return 1;
	bench("64 B write + read, 1e6", fixed);
	bench("1-256 B messages, 2 KiB in flight, 1e6", "gcc inlines the bounded copies as rep movsb", mixed);
	bench("format \"{} {}\\n\" into the ring, 1e6", "std: std::format_to through a back_inserter", formatted);
	return 0;
}
