// Benchmarks for ca::fs::path; -DUSE_STD runs the same cases on std::filesystem::path.

#include "measure.hpp"

#ifdef USE_STD
#include <filesystem>
#include <utility>

using fpath = std::filesystem::path;
static auto from(const char* const s) -> fpath { return fpath{s}.lexically_normal(); }
static auto from_clean(const char* const s) -> fpath { return fpath{s}; }
static auto join(const fpath& p, const char* const s) -> fpath { return p / s; }
static auto name_size(const fpath& p) -> size_t { return p.filename().native().size(); }
static auto stem_size(const fpath& p) -> size_t { return p.stem().native().size(); }
static auto suffix_size(const fpath& p) -> size_t { return p.extension().native().size(); }
static auto parent_size(const fpath& p) -> size_t { return p.parent_path().native().size(); }
static auto size_of(const fpath& p) -> size_t { return p.native().size(); }
static auto parts(const fpath& p) -> size_t {
	size_t sum = 0;
	for (const fpath& part : p)
		sum += part.native().size();
	return sum;
}
#else
#include "5-collections/fs/path.hpp"

using fpath = ca::fs::path;
static auto from(const char* const s) -> fpath { return ca::move(*fpath::make(s)); }
static auto from_clean(const char* const s) -> fpath { return ca::move(*fpath::make(s)); }
static auto join(const fpath& p, const char* const s) -> fpath { return ca::move(*p.join(s)); }
static auto name_size(const fpath& p) -> size_t { return p.name().size(); }
static auto stem_size(const fpath& p) -> size_t { return p.stem().size(); }
static auto suffix_size(const fpath& p) -> size_t { return p.suffix().size(); }
static auto parent_size(const fpath& p) -> size_t { return p.parent().size(); }
static auto size_of(const fpath& p) -> size_t { return p.view().size(); }
static auto parts(const fpath& p) -> size_t {
	size_t sum = 0;
	for (const ca::sv::str part : p.components())
		sum += part.size();
	return sum;
}
#endif


// -- CASES ------------------------------------------------------------------

static const int N = 1000000;

static const char* const messy[4] = {
	"//usr/./local/lib//libfoo.so.1/", "home/user/../docs/./report.final.pdf", "/var//log/syslog.2.gz", "./a/b/c/d/e/f/g.txt",
};
static const char* const clean[4] = {
	"/usr/local/lib/libfoo.so.1", "home/user/docs/report.final.pdf", "/var/log/syslog.2.gz", "a/b/c/d/e/f/g.txt",
};

NI static auto build_messy(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < N; ++i)
		sum += size_of(from(messy[i & 3]));
	sink = sum;
}

NI static auto build_clean(void) -> void {
	size_t sum = 0;
	for (int i = 0; i < N; ++i)
		sum += size_of(from_clean(clean[i & 3]));
	sink = sum;
}

NI static auto join_file(const fpath* const bases) -> void {
	size_t sum = 0;
	for (int i = 0; i < N; ++i)
		sum += size_of(join(bases[i & 3], "file.txt"));
	sink = sum;
}

NI static auto name_parts(const fpath* const bases) -> void {
	size_t sum = 0;
	for (int i = 0; i < N; ++i)
		sum += name_size(bases[i & 3]) + stem_size(bases[i & 3]) + suffix_size(bases[i & 3]);
	sink = sum;
}

NI static auto parent(const fpath* const bases) -> void {
	size_t sum = 0;
	for (int i = 0; i < N; ++i)
		sum += parent_size(bases[i & 3]);
	sink = sum;
}

NI static auto components(const fpath* const bases) -> void {
	size_t sum = 0;
	for (int i = 0; i < N; ++i)
		sum += parts(bases[i & 3]);
	sink = sum;
}


// -- MAIN -------------------------------------------------------------------

auto main(void) -> int {
	if (!open_counters())
		return 1;
	const fpath bases[4] = {from_clean(clean[0]), from_clean(clean[1]), from_clean(clean[2]), from_clean(clean[3])};
	bench("make, messy input, 1e6", "std: constructor + lexically_normal", build_messy);
	bench("make, clean input, 1e6", "std: constructor only, no normalization", build_clean);
	bench("join of a file name, 1e6", "std: operator/", [&] { join_file(bases); });
	bench("name + stem + suffix, 1e6", [&] { name_parts(bases); });
	bench("parent, 1e6", [&] { parent(bases); });
	bench("component iteration, 1e6", "libstdc++ splits components once, when constructing; ca scans on each walk", [&] { components(bases); });
	return 0;
}
