// expect: exit 0
#include "4-lifecycle/make.hpp"

class plain final {
	public:
		int value;
		explicit plain(const int v) noexcept
		: value{v} {
		}
};

class reporting final {
	public:
		int value;
		reporting(ca::error& error, const int v) noexcept
		: value{v} {
			if (v < 0)
				error = ca::error{22};
		}
};

auto main(void) -> int {
	auto a = ca::make<plain>(3);
	if (!a || a->value != 3)
		return 1;
	auto b = ca::make<reporting>(4);
	if (!b || b->value != 4)
		return 2;
	auto c = ca::make<reporting>(-1);
	return !c && c.error().code() == 22 ? 0 : 3;
}
