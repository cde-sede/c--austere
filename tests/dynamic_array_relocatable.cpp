// expect: exit 0
#include "1-primitives/debug/trap.hpp"
#include "5-collections/array/dynamic_array.hpp"

static bool relocating = false;

struct relocatable {
	static constexpr bool TRIVIALLY_RELOCATABLE = true;
	int value;
	relocatable(const int v) noexcept : value{v} {}
	relocatable(relocatable&& other) noexcept : value{other.value} { if (relocating) ca::trap(); }
	~relocatable(void) noexcept { if (relocating) ca::trap(); }
};

auto main(void) -> int {
	static_assert(ca::is_trivially_relocatable<relocatable>);
	static_assert(ca::is_trivially_relocatable<ca::dynamic_array<int>>);
	ca::dynamic_array<relocatable> a;
	for (int i = 0; i < 4; ++i) {
		if (!a.emplace(i))
			return 1;
	}
	relocating = true;
	if (!a.reserve(64) || !a.emplace(4) || a[0].value != 0 || a[4].value != 4)
		return 2;
	if (!a.shrink() || a.capacity() < 5 || a.capacity() >= 64 || a[3].value != 3)
		return 3;
	relocating = false;
	if (!a.insert(0, 9))
		return 4;
	return a[0].value == 9 && a[1].value == 0 && a[5].value == 4 && a.destroy() ? 0 : 5;
}
