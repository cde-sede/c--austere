#include "1-primitives/memory/copy_bytes.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/lstat.hpp"
#include "3-system/file/open.hpp"
#include "3-system/file/stat.hpp"
#include "3-system/file/utimens.hpp"
#include "3-system/file/write_all.hpp"
#include "3-system/fs/options.hpp"
#include "5-collections/fs/impl/path_text.hpp"


static constexpr int read_flags  = ca::fs::access::READ_ONLY | ca::fs::cloexec_t::OPEN_VALUE;
static constexpr int write_flags = ca::fs::access::WRITE_ONLY | ca::fs::create_t::OPEN_VALUE | ca::fs::truncate_t::OPEN_VALUE
	| ca::fs::cloexec_t::OPEN_VALUE;
static constexpr int touch_flags = ca::fs::access::WRITE_ONLY | ca::fs::create_t::OPEN_VALUE | ca::fs::cloexec_t::OPEN_VALUE;

static auto pop_last(ca::sv::str& rest) noexcept -> ca::sv::str {
	for (;;) {
		const char* const data = rest.data();
		ca::usz end = rest.size();
		while (end != 0U && data[end - 1U] == '/')
			--end;
		if (end == 0U) {
			rest = rest.prefix(1U);
			return ca::sv::str{};
		}
		ca::usz start = end;
		while (start != 0U && data[start - 1U] != '/')
			--start;
		const ca::sv::str component{data + start, end - start};
		rest = rest.prefix(start);
		if (component != ca::sv::str{"."})
			return component;
	}
}

static auto class_end(const ca::sv::str pattern, const ca::usz open) noexcept -> ca::usz {

	ca::usz i = open + 1U;
	if (i < pattern.size() && (pattern.data()[i] == '!' || pattern.data()[i] == '^'))
		++i;
	if (i < pattern.size() && pattern.data()[i] == ']')
		++i;
	while (i < pattern.size() && pattern.data()[i] != ']')
		++i;
	return i < pattern.size() ? i + 1U : 0U;
}

static auto class_has(const ca::sv::str pattern, const ca::usz open, const ca::usz end, const char c) noexcept -> bool {

	const char* const p = pattern.data();
	ca::usz i = open + 1U;
	const bool negated = p[i] == '!' || p[i] == '^';
	if (negated)
		++i;
	const ca::usz close = end - 1U;
	bool hit = false;
	for (ca::usz first = i; i < close; ++i) {
		if (p[i] == ']' && i != first)
			break;
		if (i + 2U < close && p[i + 1U] == '-') {
			hit = hit || (p[i] <= c && c <= p[i + 2U]);
			i += 2U;
		}
		else
			hit = hit || p[i] == c;
	}
	return hit != negated;
}


[[nodiscard]] auto ca::fs::impl::path_is_missing(const ca::error& error) noexcept -> bool {
	return error == ca::errc::no_entry || error == ca::errc::not_a_directory || error == ca::errc::symlink_loop;
}

[[nodiscard]] auto ca::fs::impl::path_is_valid_name(const ca::sv::str name) noexcept -> bool {
	return !name.empty() && name != ca::sv::str{"."} && !name.contains('/') && !name.contains('\0');
}

[[nodiscard]] auto ca::fs::impl::path_append_components(char* const out, ca::usz size, const ca::sv::str raw) noexcept -> ca::usz {

	const char* at = raw.begin();
	const char* const end = raw.end();
	if (at != end && *at == '/') {
		out[0] = '/';
		  size = 1U;
	}
	while (at != end) {
		if (*at == '/') {
			++at;
			continue;
		}
		const char* const start = at;
		while (at != end && *at != '/')
			++at;
		const ca::usz length = static_cast<ca::usz>(at - start);
		if (length == 1U && *start == '.')
			continue;
		if (size != 0U && !(size == 1U && out[0] == '/'))
			out[size++] = '/';
		ca::copy_bytes(out + size, start, length);
		size += length;
	}
	return size;
}

[[nodiscard]] auto ca::fs::impl::path_pop_first(ca::sv::str& rest) noexcept -> ca::sv::str {
	for (;;) {
		const char* at = rest.begin();
		while (at != rest.end() && *at == '/')
			++at;
		const char* stop = at;
		while (stop != rest.end() && *stop != '/')
			++stop;
		const ca::sv::str component{at, static_cast<ca::usz>(stop - at)};
		rest = ca::sv::str{stop, static_cast<ca::usz>(rest.end() - stop)};
		if (component != ca::sv::str{"."})
			return component;
	}
}

[[nodiscard]] auto ca::fs::impl::path_has_magic(const ca::sv::str pattern) noexcept -> bool {
	return pattern.contains('*') || pattern.contains('?') || pattern.contains('[');
}

[[nodiscard]] auto ca::fs::impl::path_glob_matches(const ca::sv::str pattern, const ca::sv::str name) noexcept -> bool {

	const char* const p = pattern.data();
	const char* const s = name.data();
	ca::usz pi = 0U;
	ca::usz si = 0U;
	ca::usz star = pattern.size();
	ca::usz resume = 0U;
	while (si < name.size()) {
		bool advanced = false;
		if (pi < pattern.size()) {
			if (p[pi] == '*') {
				  star = pi++;
				resume = si;
				continue;
			}
			if (p[pi] == '[') {
				const ca::usz end = class_end(pattern, pi);
				if (end != 0U) {
					if (class_has(pattern, pi, end, s[si])) {
						pi = end;
						++si;
						advanced = true;
					}
				}
				else if (s[si] == '[') {
					++pi;
					++si;
					advanced = true;
				}
			}
			else if (p[pi] == '?' || p[pi] == s[si]) {
				++pi;
				++si;
				advanced = true;
			}
		}
		if (advanced)
			continue;
		if (star == pattern.size())
			return false;
		pi = star + 1U;
		si = ++resume;
	}
	while (pi < pattern.size() && p[pi] == '*')
		++pi;
	return pi == pattern.size();
}

[[nodiscard]] auto ca::fs::impl::path_match(ca::sv::str text, const ca::sv::str pattern) noexcept -> bool {

	if (pattern.empty())
		return false;
	const bool anchored = pattern.data()[0] == '/';
	ca::sv::str globs = pattern;
	for (;;) {
		const ca::sv::str glob = pop_last(globs);
		if (glob.empty())
			break;
		const ca::sv::str component = pop_last(text);
		if (component.empty() || !ca::fs::impl::path_glob_matches(glob, component))
			return false;
	}
	return !anchored || text == ca::sv::str{"/", 1U};
}

[[nodiscard]] auto ca::fs::impl::path_relative_to(const ca::sv::str text, const ca::sv::str base) noexcept -> ca::optional<ca::sv::str> {

	if (base.empty()) {
		if (!text.empty() && text.data()[0] == '/')
			return ca::nullopt;
		return text.empty() ? ca::sv::str{".", 1U} : text;
	}
	if (!text.starts_with(base))
		return ca::nullopt;
	if (text.size() == base.size())
		return ca::sv::str{".", 1U};
	if (base.size() == 1U && base.data()[0] == '/')
		return text.without_prefix(1U);
	if (text.data()[base.size()] != '/')
		return ca::nullopt;
	return text.without_prefix(base.size() + 1U);
}

[[nodiscard]] auto ca::fs::impl::path_exists(const char* const path) noexcept -> ca::result<bool> {

	auto status = ca::sys::stat(path);
	if (status)
		return true;
	if (ca::fs::impl::path_is_missing(status.error()))
		return false;
	return status.propagate();
}

[[nodiscard]] auto ca::fs::impl::path_kind_is(const char* const path, const bool follow, const ca::sys::file_kind kind) noexcept -> ca::result<bool> {

	auto status = follow ? ca::sys::stat(path) : ca::sys::lstat(path);
	if (status)
		return status->kind() == kind;
	if (ca::fs::impl::path_is_missing(status.error()))
		return false;
	return status.propagate();
}

[[nodiscard]] auto ca::fs::impl::path_touch(const char* const path, const ca::u32 mode) noexcept -> ca::result<void> {

	auto touched = ca::sys::utimens(path);
	if (touched || touched.error() != ca::errc::no_entry)
		return touched;
	auto fd = ca::sys::open(path, touch_flags, mode);
	if (!fd)
		return fd.propagate();
	return ca::sys::close(*fd);
}

[[nodiscard]] auto ca::fs::impl::path_write(const char* const path, const ca::span<const ca::byte> bytes) noexcept -> ca::result<void> {

	auto fd = ca::sys::open(path, write_flags, 0666U);
	if (!fd)
		return fd.propagate();
	auto written = ca::sys::write_all(*fd, bytes);
	auto closed = ca::sys::close(*fd);
	if (!written)
		return written;
	return closed;
}

[[nodiscard]] auto ca::fs::impl::path_open_read(const char* const path) noexcept -> ca::result<int> {
	return ca::sys::open(path, read_flags, 0U);
}
