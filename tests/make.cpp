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

static int acquired = 0;

class factory final {
	private:
		explicit factory(const int v) noexcept
		: value{v} {
		}

	public:
		int value;

		factory(ca::error& error, const int v) noexcept
		: value{v} {
			error = ca::error{1};
		}

		[[nodiscard]] static auto make(const int v) noexcept -> ca::result<factory> {
			if (v < 0)
				return ca::failure{ca::error{34}};
			++acquired;
			return factory{v};
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
	if (c || c.error().code() != 22)
		return 3;
	auto d = ca::make<factory>(5);
	if (!d || d->value != 5 || acquired != 1)
		return 4;
	auto e = ca::make<factory>(-5);
	return !e && e.error().code() == 34 && acquired == 1 ? 0 : 5;
}
