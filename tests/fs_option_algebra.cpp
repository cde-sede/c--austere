// expect: exit 0
#include "1-primitives/traits/is_same.hpp"
#include "3-system/fs/options.hpp"

template <typename A, typename B>
inline constexpr bool same = ca::is_same<A, B>;

constexpr auto a = ca::fs::append;
constexpr auto n = ca::fs::nonblock;
constexpr auto c = ca::fs::cloexec;

static_assert(same<decltype(a | n | a), ca::option_set<ca::fs::append_t, ca::fs::nonblock_t>>);
static_assert(same<decltype((a | n) & (n | c)), ca::option_set<ca::fs::nonblock_t>>);
static_assert(same<decltype((a | n | c) & ~n), ca::option_set<ca::fs::append_t, ca::fs::cloexec_t>>);
static_assert(same<decltype(~n & (a | n)), ca::option_set<ca::fs::append_t>>);
static_assert(same<decltype(~~a), ca::option_set<ca::fs::append_t>>);
static_assert(same<decltype(~a | ~n), decltype(~(a & n))>);
static_assert(same<decltype(~a & ~n), decltype(~(a | n))>);
static_assert(decltype(a | n)::HAS<ca::fs::nonblock_t> && !decltype(a | n)::HAS<ca::fs::cloexec_t>);
static_assert(!decltype(c | ca::fs::clofork)::SUPPORTED || ca::fs::has_clofork);

auto main(void) -> int {
	return 0;
}
