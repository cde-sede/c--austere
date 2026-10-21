// expect: exit 0
#include "4-lifecycle/make.hpp"
#include "4-lifecycle/unique.hpp"

static int acquired = 0;

class factory final {
	private:
		explicit factory(const int v) noexcept
		: value{v} {
		}

	public:
		int value;

		[[nodiscard]] static auto make(const int v) noexcept -> ca::result<factory> {
			if (v < 0)
				return ca::failure{ca::error{34}};
			++acquired;
			return factory{v};
		}
};

auto main(void) -> int {
	auto made = ca::unique<factory>::make(3);
	if (!made || (*made)->value != 3 || acquired != 1)
		return 1;
	auto failed = ca::unique<factory>::make(-3);
	if (failed || failed.error().code() != 34 || acquired != 1)
		return 2;
	auto through = ca::make<ca::unique<factory>>(4);
	if (!through || (*through)->value != 4 || acquired != 2)
		return 3;
	auto boxed = ca::make<ca::unique<int>>(5);
	if (!boxed || **boxed != 5)
		return 4;
	return made->cleanup() && through->cleanup() && boxed->cleanup() ? 0 : 5;
}
