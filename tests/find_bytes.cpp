// expect: exit 0
#include "1-primitives/memory/find_byte.hpp"
#include "1-primitives/memory/find_unit.hpp"
#include "1-primitives/memory/impl/cpu_features.hpp"
#include "1-primitives/memory/impl/find_bytes.hpp"
#include "1-primitives/memory/string_length.hpp"

extern "C" auto memchr(const void* data, int value, ca::usz count) noexcept -> void*;
extern "C" auto strlen(const char* text) noexcept -> ca::usz;

static constexpr ca::usz LARGE = (ca::usz{1} << 20U) + 4096U;

static char work[LARGE + 256U];

static auto (*volatile memchr_c)(const void*, int, ca::usz) noexcept -> void* = &memchr;
static auto (*volatile strlen_c)(const char*) noexcept -> ca::usz = &strlen;

static const ca::usz offsets[] = {0U, 1U, 7U, 15U, 16U, 17U, 31U, 33U, 63U};
static const ca::usz large[] = {1023U, 4095U, 4097U, 65536U + 3U, ca::usz{1} << 20U};
static const int needles[] = {0, 'x', 0x80, 0xff, 0x100 + 'x', -1};

/* count bytes at work + 64 + offset without the needle, the needle just before and just after
   them, then the needle at each of positions: the first one is found, none past count. */
static auto check_find(const ca::impl::find_byte_function find, const ca::usz count, const ca::usz offset, const int value) -> bool {
	const char needle = static_cast<char>(value);
	const char other = static_cast<char>(needle ^ 0x55);
	char* const data = work + 64U + offset;
	for (ca::usz i = 0; i < count + 128U; ++i)
		work[i] = other;
	data[-1] = needle;
	data[count] = needle;
	if (find(data, value, count) != nullptr)
		return false;
	const ca::usz positions[] = {0U, count / 2U, count - 1U, count > 40U ? count - 40U : 0U};
	const ca::usz every = count <= 140U ? count : 0U;
	for (ca::usz k = 0; count != 0U && k < every + 4U; ++k) {
		const ca::usz at = k < every ? k : positions[k - every];
		data[at] = needle;
		if (at + 1U < count)
			data[count - 1U] = needle;
		const void* const got = find(data, value, count);
		data[at] = other;
		data[count - 1U] = other;
		if (got != data + at)
			return false;
	}
	return true;
}

/* A string of length at work + 64 + offset, NULs before it. */
static auto check_length(const ca::impl::string_length_function length_of, const ca::usz length, const ca::usz offset) -> bool {
	char* const text = work + 64U + offset;
	for (ca::usz i = 0; i < 64U + offset; ++i)
		work[i] = 0;
	for (ca::usz i = 0; i < length; ++i)
		text[i] = static_cast<char>(0x80 | (i % 127U + 1U));
	text[length] = 0;
	text[length + 1U] = 'y';
	return length_of(text) == length;
}

static auto all_find(const ca::impl::find_byte_function find) -> bool {
	for (const ca::usz offset : offsets) {
		for (const int value : needles) {
			for (ca::usz count = 0U; count <= 300U; ++count)
				if (!check_find(find, count, offset, value))
					return false;
		}
		for (const ca::usz count : large)
			if (!check_find(find, count, offset, 'x'))
				return false;
	}
	return true;
}

static auto all_length(const ca::impl::string_length_function length_of) -> bool {
	for (const ca::usz offset : offsets) {
		for (ca::usz length = 0U; length <= 700U; ++length)
			if (!check_length(length_of, length, offset))
				return false;
		for (const ca::usz length : large)
			if (!check_length(length_of, length, offset))
				return false;
	}
	return true;
}

static auto find_builtin(const void* const data, const int value, const ca::usz count) noexcept -> void* {
	return const_cast<char*>(ca::find_byte(static_cast<const char*>(data), static_cast<char>(value), count));
}

static auto unit_builtin(const void* const data, const int value, const ca::usz count) noexcept -> void* {
	return const_cast<char*>(ca::find_unit(static_cast<const char*>(data), static_cast<char>(value), count));
}

static auto length_builtin(const char* const text) noexcept -> ca::usz {
	return ca::string_length(text);
}

static_assert(ca::string_length("hello") == 5U);

auto main(void) -> int {
	if (!all_find(memchr_c) || !all_find(&find_builtin) || !all_find(&unit_builtin))
		return 1;
	if (!all_length(strlen_c) || !all_length(&length_builtin))
		return 2;
	if (!all_find(&ca::impl::find_byte_sse2) || !all_length(&ca::impl::string_length_sse2))
		return 3;
	const ca::impl::cpu_features cpu = ca::impl::read_cpu_features();
	if (cpu._avx2 && cpu._bmi2 && (!all_find(&ca::impl::find_byte_avx2) || !all_length(&ca::impl::string_length_avx2)))
		return 4;
	return memchr_c(nullptr, 0, 0U) == nullptr ? 0 : 5;
}
