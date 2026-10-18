// expect: trap
#include "5-collections/list/intrusive_list.hpp"

struct item : ca::list_link<item> {
	int value;
};

auto main(void) -> int {
	item one{};
	ca::intrusive_list<item> first;
	ca::intrusive_list<item> second;
	first.append(one);
	second.append(one);
	return 0;
}
