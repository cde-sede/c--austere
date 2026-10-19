// expect: exit 0
#include "1-primitives/traits/is_trivially_destructible.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "4-lifecycle/unique.hpp"
#include "5-collections/array/deque.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/array/inline_array.hpp"
#include "5-collections/array/slice.hpp"
#include "5-collections/array/small_array.hpp"
#include "5-collections/fs/path.hpp"
#include "5-collections/list/list.hpp"
#include "5-collections/map/hashmap.hpp"
#include "5-collections/map/map.hpp"
#include "5-collections/map/set.hpp"
#include "5-collections/string/str.hpp"
#include "5-collections/trace/report.hpp"
#include "6-resources/fs/descriptor.hpp"
#include "6-resources/io/buffered_reader.hpp"
#include "6-resources/io/buffered_writer.hpp"
#include "6-resources/io/writer.hpp"

struct nontrivial {
	~nontrivial(void) noexcept {}
};

static_assert(ca::is_trivially_destructible<ca::optional<int>>);
static_assert(ca::is_trivially_destructible<ca::optional<int&>>);
static_assert(ca::is_trivially_destructible<ca::optional<ca::optional<int>>>);
static_assert(ca::is_trivially_destructible<ca::result<int>>);
static_assert(ca::is_trivially_destructible<ca::result<void>>);
static_assert(!ca::is_trivially_destructible<ca::optional<nontrivial>>);
static_assert(!ca::is_trivially_destructible<ca::result<nontrivial>>);

static_assert(ca::is_trivially_destructible<ca::arena>);
static_assert(ca::is_trivially_destructible<ca::unique<int>>);
static_assert(ca::is_trivially_destructible<ca::deque<int>>);
static_assert(ca::is_trivially_destructible<ca::dynamic_array<int>>);
static_assert(ca::is_trivially_destructible<ca::dynamic_array<ca::str>>);
static_assert(ca::is_trivially_destructible<ca::inline_array<int, 4>>);
static_assert(ca::is_trivially_destructible<ca::slice<int>>);
static_assert(ca::is_trivially_destructible<ca::small_array<int, 4>>);
static_assert(ca::is_trivially_destructible<ca::fs::path>);
static_assert(ca::is_trivially_destructible<ca::list<int>>);
static_assert(ca::is_trivially_destructible<ca::hashmap<int, int>>);
static_assert(ca::is_trivially_destructible<ca::map<int, int>>);
static_assert(ca::is_trivially_destructible<ca::set<int>>);
static_assert(ca::is_trivially_destructible<ca::str>);
static_assert(ca::is_trivially_destructible<ca::report<>>);
static_assert(ca::is_trivially_destructible<ca::fs::descriptor>);
static_assert(ca::is_trivially_destructible<ca::io::buffered_reader<>>);
static_assert(ca::is_trivially_destructible<ca::io::buffered_writer<>>);
static_assert(ca::is_trivially_destructible<ca::io::writer>);

auto main(void) -> int {
	return 0;
}
