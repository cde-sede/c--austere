// Benchmarks for ca formatting; -DUSE_STD runs the same cases on snprintf and fprintf.

#include "measure.hpp"

#include <fcntl.h>

#ifdef USE_STD
#include <stdio.h>

static char text[256];
static FILE* null_file = nullptr;

#define FORMAT(...) static_cast<size_t>(snprintf(text, sizeof text, __VA_ARGS__))
#define PRINT(...) fprintf(null_file, __VA_ARGS__)
#define BRACES(braces, percent, ...) FORMAT(percent, __VA_ARGS__)

static auto open_null(void) -> bool {
	null_file = fopen("/dev/null", "w");
	return null_file != nullptr;
}
#else
#include "5-collections/format/format_to.hpp"
#include "5-collections/format/printf_to.hpp"
#include "5-collections/io/memory_writer.hpp"
#include "6-resources/io/writer.hpp"
#include "6-resources/print/printf.hpp"

static char text[256];
static char null_buffer[4096];
static ca::io::writer null_stream{};

#define FORMAT(...) [&] { ca::io::memory_writer m{text}; return static_cast<size_t>(ca::printf_to(m, __VA_ARGS__).value_or(0U)); }()
#define PRINT(...) static_cast<void>(ca::printf(null_stream, __VA_ARGS__))
#define BRACES(braces, percent, ...) [&] { ca::io::memory_writer m{text}; return static_cast<size_t>(ca::format_to(m, braces, __VA_ARGS__).value_or(0U)); }()

static auto open_null(void) -> bool {
	const int fd = open("/dev/null", O_WRONLY);
	if (fd < 0)
		return false;
	ca::construct_at(&null_stream, fd, ca::io::buffering::full, null_buffer);
	return true;
}
#endif


// -- CASES ------------------------------------------------------------------

static const int N = 1000000;

static const char* const names[4] = {"alpha", "beta", "gamma", "delta"};

NI static auto integers(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < N; ++i)
		sum += FORMAT("%d %d %u\n", i, -i, static_cast<unsigned>(i) * 2654435761U);
	sink = sum;
}

NI static auto integers_braces(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < N; ++i)
		sum += BRACES("{} {} {}\n", "%d %d %u\n", i, -i, static_cast<unsigned>(i) * 2654435761U);
	sink = sum;
}

NI static auto padded(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < N; ++i)
		sum += FORMAT("%-8s|%6d|%08x\n", names[i & 3], i, static_cast<unsigned>(i));
	sink = sum;
}

NI static auto strings(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < N; ++i)
		sum += FORMAT("%s=%s; %s\n", names[i & 3], names[(i + 1) & 3], "a longer literal tail of text");
	sink = sum;
}

NI static auto fixed_floats(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < N; ++i)
		sum += FORMAT("%.3f\n", static_cast<double>(i) * 0.37);
	sink = sum;
}

NI static auto shortest_floats(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < N; ++i)
		sum += BRACES("{}\n", "%.17g\n", static_cast<double>(i) * 0.37);
	sink = sum;
}

NI static auto buffered_file(void) -> void {
	for (int i = 0; i < N; ++i)
		PRINT("line %d of %s\n", i, names[i & 3]);
}


// -- MAIN -------------------------------------------------------------------

auto main(void) -> int {
	if (!open_counters() || !open_null())
		return 1;
	bench("3 integers, printf_to, 1e6", "std: snprintf", integers);
	bench("3 integers, format_to, 1e6", "std: snprintf with the % spelling", integers_braces);
	bench("%-8s %6d %08x, 1e6", padded);
	bench("3 strings, 1e6", strings);
	bench("%.3f, 1e6", "ca: exact digits, no libc", fixed_floats);
	bench("shortest float {}, 1e6", "ca: Ryu; std: %.17g, which is not the shortest round trip", shortest_floats);
	bench("buffered fd line, ca::printf, 1e6", "std: fprintf to /dev/null", buffered_file);
	return 0;
}
