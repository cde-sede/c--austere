// Benchmarks for ca readers; -DUSE_STD runs the same cases on stdio (getline, fread).

#include "measure.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static const char* const path = "build/bench/read_input.txt";
static const int LINES = 1000000;

static auto make_input(void) -> bool {
	FILE* const f = fopen(path, "w");
	if (f == nullptr)
		return false;
	for (int i = 0; i < LINES; ++i)
		fprintf(f, "%d,%s,%x\n", i, "some field text of a log line" + (i % 20), static_cast<unsigned>(i) * 2654435761U);
	return fclose(f) == 0;
}

#ifdef USE_STD
#include <stdlib.h>

NI static auto lines_view(void) -> void {
	FILE* const f = fopen(path, "r");
	char* line = nullptr;
	size_t capacity = 0;
	size_t sum = 0;
	ssize_t got = 0;
	while ((got = getline(&line, &capacity, f)) > 0)
		sum += static_cast<size_t>(got);
	free(line);
	fclose(f);
	sink = sum;
}

NI static auto lines_copy(void) -> void {
	lines_view();
}

NI static auto bulk(void) -> void {
	FILE* const f = fopen(path, "r");
	char chunk[4096];
	size_t sum = 0;
	size_t got = 0;
	while ((got = fread(chunk, 1, sizeof chunk, f)) > 0)
		sum += got + static_cast<unsigned char>(chunk[0]);
	fclose(f);
	sink = sum;
}
#else
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/io/read_line.hpp"
#include "6-resources/io/reader.hpp"

static char buffer[65536];

NI static auto lines_view(void) -> void {
	const int fd = open(path, O_RDONLY);
	ca::io::reader r{fd, buffer};
	size_t sum = 0;
	for (;;) {
		auto line = r.read_until('\n');
		if (!line || line->empty())
			break;
		sum += line->size();
	}
	close(fd);
	sink = sum;
}

NI static auto lines_copy(void) -> void {
	const int fd = open(path, O_RDONLY);
	ca::io::reader r{fd, buffer};
	ca::dynamic_array<char> line;
	size_t sum = 0;
	for (;;) {
		auto got = ca::io::read_line(r, line);
		if (!got || !*got)
			break;
		sum += line.size() + 1U;
	}
	static_cast<void>(line.destroy());
	close(fd);
	sink = sum;
}

NI static auto bulk(void) -> void {
	const int fd = open(path, O_RDONLY);
	ca::io::reader r{fd, ca::span{buffer, 4096U}};
	char chunk[4096];
	size_t sum = 0;
	for (;;) {
		auto got = r.read(chunk);
		if (!got || *got == 0U)
			break;
		sum += *got + static_cast<unsigned char>(chunk[0]);
	}
	close(fd);
	sink = sum;
}
#endif


// -- MAIN -------------------------------------------------------------------

auto main(void) -> int {
	if (!make_input() || !open_counters())
		return 1;
	bench("1e6 lines in place, read_until", "std: getline", lines_view);
	bench("1e6 lines copied, read_line", "std: getline", lines_copy);
	bench("4 KiB chunks, read", "std: fread", bulk);
	return 0;
}
