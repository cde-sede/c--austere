// expect: exit 0
#include "1-primitives/traits/is_same.hpp"
#include "4-lifecycle/make_in.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "4-lifecycle/memory/page_allocator.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/fs/path.hpp"

using arena_array = ca::dynamic_array<int, ca::arena_allocator>;
using arena_path  = ca::fs::basic_path<ca::arena_allocator>;

static_assert(ca::is_same<decltype(ca::make_in<ca::dynamic_array<int>>(ca::declval<ca::arena&>())), ca::result<arena_array>>);
static_assert(ca::is_same<decltype(ca::make_in<ca::fs::path>(ca::declval<ca::arena&>(), ca::sv::str{})), ca::result<arena_path>>);
static_assert(ca::is_same<decltype(ca::make_in<ca::fs::path>(ca::page_allocator{}, ca::sv::str{})), ca::result<ca::fs::basic_path<ca::page_allocator>>>);
static_assert(sizeof(ca::fs::path) == 2U * sizeof(void*));
static_assert(sizeof(arena_path) == 3U * sizeof(void*));

auto main(void) -> int {

	auto built = ca::arena::make();
	if (!built)
		return 1;
	ca::arena arena = ca::move(*built);
	const ca::arena_allocator allocator = arena.allocator();

	{
		auto array = ca::make_in<ca::dynamic_array<int>>(arena);
		if (!array || !array->append(3) || !array->append(4) || (*array)[1] != 4)
			return 2;

		auto made = ca::make_in<ca::fs::path>(arena, ca::sv::str{"/tmp//x/./y"});
		if (!made || made->view() != ca::sv::str{"/tmp/x/y"})
			return 4;
		auto joined = made->join(ca::sv::str{"z.txt"});
		if (!joined || joined->view() != ca::sv::str{"/tmp/x/y/z.txt"})
			return 5;
		auto renamed = joined->with_suffix(ca::sv::str{".md"});
		if (!renamed || renamed->name() != ca::sv::str{"z.md"})
			return 6;

		auto root = arena_path::make_in(allocator, ca::sv::str{"/"});
		if (!root)
			return 7;
		auto entries = root->iterdir();
		if (!entries || entries->empty())
			return 8;
		auto copy = (*entries)[0].clone();
		if (!copy || !(*copy == (*entries)[0]))
			return 9;
		auto here = arena_path::cwd(allocator);
		if (!here || !here->is_absolute())
			return 10;
		ca::fs::path heap_path;
		if (!(heap_path == arena_path{allocator}) || !made->is_relative_to(*root))
			return 11;

		if (!entries->cleanup() || !array->cleanup())
			return 12;
	}
	return arena.cleanup() ? 0 : 13;
}
