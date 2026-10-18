// expect: trap
#include "5-collections/list/intrusive_list.hpp"

struct item : ca::list_link<item> {
	int value;
};

auto main(void) -> int {
	ca::intrusive_list<item> list;
	{
		item scoped{};
		list.append(scoped);
	}
	return static_cast<int>(list.size());
}
