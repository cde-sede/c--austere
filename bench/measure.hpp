// Measurement harness shared by the benchmarks: each case reports the minimum
// over 9 repetitions of user-space cycles and instructions (perf_event_open),
// and wall time.
//
// Output, one tab-separated row per case after a header row:
//   section  case  cycles  instructions  ms  note
// bench(name, note, f) names a case and says what its figure shows; section(name)
// groups the cases after it, so a header can quote one group (--BENCH bench/x.cpp
// group, see tooling/autodoc.py --bench). Cases and notes are the same in the
// -DUSE_STD build, which is the baseline every other build is divided by.
//
// A bench declares its builds besides the baseline in comment lines:
//   // variant: LABEL [-DFLAG...]
// one column per variant and compiler; none declared means one variant, ca.

#ifndef ca_bench_measure_hpp
#define ca_bench_measure_hpp

#include <linux/perf_event.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#define NI __attribute__((noinline))


// -- MEASUREMENT ------------------------------------------------------------

static int counters[2] = {-1, -1};

static auto open_counter(const uint64_t config, const int group) -> int {
	perf_event_attr attr;
	memset(&attr, 0, sizeof attr);
	attr.size           = sizeof attr;
	attr.type           = PERF_TYPE_HARDWARE;
	attr.config         = config;
	attr.disabled       = group == -1;
	attr.exclude_kernel = 1;
	attr.exclude_hv     = 1;
	attr.read_format    = PERF_FORMAT_GROUP;
	return static_cast<int>(syscall(SYS_perf_event_open, &attr, 0, -1, group, 0));
}

static auto now(void) -> double {
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return static_cast<double>(t.tv_sec) * 1e3 + static_cast<double>(t.tv_nsec) / 1e6;
}

static volatile uint64_t sink;

static const char* current_section = "";

static auto section(const char* const name) -> void {
	current_section = name;
}

template <typename F>
static auto bench(const char* const name, const char* const note, F f) -> void {
	double ms = 1e30;
	uint64_t cycles = UINT64_MAX;
	uint64_t instructions = UINT64_MAX;
	for (int rep = 0; rep < 9; ++rep) {
		ioctl(counters[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
		const double t0 = now();
		ioctl(counters[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
		f();
		ioctl(counters[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
		const double t1 = now();
		uint64_t values[3] = {};
		if (read(counters[0], values, sizeof values) != sizeof values)
			__builtin_trap();
		if (values[1] < cycles)
			cycles = values[1];
		if (values[2] < instructions)
			instructions = values[2];
		if (t1 - t0 < ms)
			ms = t1 - t0;
	}
	printf("%s\t%s\t%llu\t%llu\t%.2f\t%s\n", current_section, name, static_cast<unsigned long long>(cycles),
		static_cast<unsigned long long>(instructions), ms, note);
	fflush(stdout);
}

template <typename F>
static auto bench(const char* const name, F f) -> void {
	bench(name, "", f);
}


static auto open_counters(void) -> bool {
	counters[0] = open_counter(PERF_COUNT_HW_CPU_CYCLES, -1);
	counters[1] = open_counter(PERF_COUNT_HW_INSTRUCTIONS, counters[0]);
	if (counters[0] < 0 || counters[1] < 0) {
		perror("perf_event_open (kernel.perf_event_paranoid <= 2?)");
		return false;
	}
	printf("section\tcase\tcycles\tinstructions\tms\tnote\n");
	return true;
}

#endif // ca_bench_measure_hpp
