// expect: exit 0
#include "2-vocabulary/fail.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"

auto main(void) -> int {
	const ca::optional<int> some = 21;
	const ca::optional<int> none;
	const ca::result<int> failed = ca::fail(5);
	const int a = some.match([](const ca::realized<int>& v) { return *v * 2; }, [] { return -1; });
	const int b = none.match([](const ca::realized<int>& v) { return *v; }, [] { return -1; });
	const int c = failed.match([](const ca::realized<int>& v) { return *v; }, [](const ca::error& e) { return e.code(); });
	const ca::realized<int> d = none.realize_or(7);
	const ca::realized<int> e = some.realize_or(7);
	if (a != 42 || b != -1 || c != 5 || *d != 7 || *e != 21)
		return 1;
	return 0;
}
