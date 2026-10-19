// expect: exit 0
#include "5-collections/array/dynamic_array.hpp"

struct node {
	int id;
	ca::dynamic_array<node> children;
};

auto main(void) -> int {
	node root{0, {}};
	if (!root.children.append(node{1, {}}))
		return 1;
	if (!root.children[0].children.append(node{2, {}}))
		return 2;
	return root.children.cleanup() ? 0 : 3;
}
