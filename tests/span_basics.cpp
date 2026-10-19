// expect: exit 0
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "5-collections/array/dynamic_array.hpp"

static_assert(__is_trivially_copyable(ca::span<int>) && sizeof(ca::span<int>) == 2U * sizeof(void*));

static auto bytes_of(const ca::span<const ca::byte> bytes) noexcept -> ca::usz {
	return bytes.size();
}

static auto fill(const ca::span<ca::byte> buffer) noexcept -> ca::usz {

	for (ca::byte& b : buffer)
		b = 'z';
	return buffer.size();
}

constexpr auto cut(void) noexcept -> bool {

	int values[5] = {1, 2, 3, 4, 5};
	const ca::span<int> all{values};
	return all.prefix(2U).size() == 2U && all.suffix(9U).size() == 5U && all.without_prefix(4U)[0] == 5
		&& all.without_suffix(7U).empty() && all.sub(1U, 2U)[1] == 3 && all.sub(9U, 2U).empty() && all.size_bytes() == 5U * sizeof(int);
}

static_assert(cut());

auto main(void) -> int {
	if (bytes_of(ca::sv::str{"hello"}) != 5U)
		return 1;
	char buffer[8];
	if (fill(buffer) != 8U || buffer[7] != 'z')
		return 2;
	ca::dynamic_array<int> a;
	if (!a.append(1) || !a.append(2))
		return 3;
	const ca::span<int> view = a;
	view[1] = 20;
	const ca::span<const int> constant = view;
	if (a[1] != 20 || constant.bytes().size() != 2U * sizeof(int) || view.writable_bytes().size() != 8U)
		return 4;
	if (view.get(2U) || view.get(1U).value_or(0) != 20)
		return 5;
	const ca::span<const ca::byte> text_bytes = ca::span{"abc", 3U};
	if (text_bytes[2] != 'c' || ca::span<int>{}.data() != nullptr)
		return 6;
	return a.cleanup() ? 0 : 7;
}
