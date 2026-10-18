// expect: exit 0
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/unique.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "4-lifecycle/memory/heap.hpp"

static int alive = 0;
static int cleaned = 0;

class tracked final {
	public:
		int value;
		tracked(ca::error& error, const int v) noexcept
		: value{v} {
			++alive;
			if (v < 0)
				error = ca::error{22};
		}
		tracked(tracked&& other) noexcept
		: value{other.value} {
			++alive;
		}
		~tracked(void) noexcept {
			--alive;
		}
		[[nodiscard]] auto destroy(void) noexcept -> ca::result<void> {
			++cleaned;
			return ca::result<void>{};
		}
		[[nodiscard]] auto clone(void) const noexcept -> ca::result<tracked> {
			ca::error error{};
			return tracked{error, value + 100};
		}
};

class alignas(64) wide final {
	public:
		char bytes[64];
};

static_assert(sizeof(ca::unique<int>) == sizeof(void*));
static_assert(sizeof(ca::unique<int, ca::arena_allocator>) == 2U * sizeof(void*));

auto main(void) -> int {

	auto made = ca::make_in<ca::unique<tracked>>(ca::heap{}, 7);
	if (!made || (*made)->value != 7 || alive != 1)
		return 1;
	ca::unique<tracked> owner = ca::move(*made);
	auto copy = owner.clone();
	if (!copy || (*copy)->value != 107 || alive != 2)
		return 2;
	if (!owner.destroy() || owner.holds() || alive != 1 || cleaned != 1)
		return 3;
	if (!copy->destroy() || alive != 0)
		return 4;
	auto failed = ca::make_in<ca::unique<tracked>>(ca::heap{}, -1);
	if (failed || failed.error().code() != 22 || alive != 0)
		return 5;

	auto built = ca::arena::make();
	if (!built)
		return 6;
	ca::arena arena = ca::move(*built);
	auto in_arena = ca::make_in<ca::unique<tracked>>(arena, 9);
	if (!in_arena || (*in_arena)->value != 9)
		return 7;
	ca::unique<tracked, ca::arena_allocator> arena_owner = ca::move(*in_arena);
	ca::unique<tracked, ca::arena_allocator> empty_owner{arena.allocator()};
	if (empty_owner.holds() || !arena_owner.destroy() || alive != 0)
		return 8;
	auto aligned = ca::make_in<ca::unique<wide>>(ca::heap{});
	if (!aligned || (reinterpret_cast<ca::usz>(aligned->get()) & 63U) != 0U)
		return 9;
	if (!aligned->destroy())
		return 10;
	return arena.destroy() ? 0 : 11;
}
