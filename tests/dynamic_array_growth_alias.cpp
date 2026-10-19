// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

struct move_only {
	int value;
	move_only(const int v) noexcept : value{v} {}
	move_only(const move_only&) = delete;
	move_only(move_only&&) noexcept = default;
	auto operator=(move_only&&) noexcept -> move_only& = default;
};

auto main(void) -> int {
	ca::dynamic_array<int> a;
	for (int i = 0; i < 100; ++i) {
		if (!a.append(a.empty() ? 7 : a[a.size() - 1]))
			return 1;
	}
	if (a.size() != 100 || a[99] != 7 || !a.shrink() || a.capacity() < 100)
		return 2;
	if (!a.resize(1000, a[50]) || a[999] != 7 || a.capacity() < 1000)
		return 3;
	if (!a.resize(3000) || a[2999] != 0 || a[999] != 7)
		return 4;
	if (!a.cleanup())
		return 5;

	ca::dynamic_array<move_only> m;
	for (int i = 0; i < 9; ++i) {
		if (!m.emplace(i))
			return 6;
	}
	if (!m.append(move_only{9}) || m[9].value != 9 || m[4].value != 4)
		return 7;

	ca::dynamic_array<ca::dynamic_array<int>> n;
	if (!n.resize(5) || !n[4].append(1) || !n.resize(50) || n[4][0] != 1 || !n[49].empty())
		return 8;
	return m.cleanup() && n.cleanup() ? 0 : 9;
}
