// expect: trap
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/unique.hpp"

class cleanable final {
	public:
		[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {
			return ca::result<void>{};
		}
};

auto main(void) -> int {
	auto made = ca::make_in<ca::unique<cleanable>>(ca::heap{});
	if (!made)
		return 1;
	ca::unique<cleanable> owner = ca::move(*made);
	owner = ca::unique<cleanable>{};
	return 0;
}
