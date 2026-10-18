// expect: exit 0
#include "5-collections/array/array.hpp"
#include "5-collections/list/intrusive_list.hpp"

struct by_age;

struct task : ca::list_link<task>, ca::list_link<task, by_age> {
	int id;
	int age;

	explicit task(const int identifier, const int years) noexcept
	: id{identifier}, age{years} {
	}
};

static_assert(ca::intrusive_list<task>::TRIVIALLY_RELOCATABLE && sizeof(ca::intrusive_list<task>) == 3U * sizeof(void*));

template <typename L, ca::usz N>
static auto ids(const L& list, const ca::array<int, N>& expected) noexcept -> bool {

	if (list.size() != N)
		return false;
	ca::usz i = 0U;
	for (const task& t : list) {
		if (t.id != expected[i++])
			return false;
	}
	i = N;
	for (auto at = list.end(); at != list.begin();) {
		--at;
		if (at->id != expected[--i])
			return false;
	}
	return true;
}

auto main(void) -> int {

	task a{1, 30};
	task b{2, 10};
	task c{3, 20};
	task d{4, 10};
	ca::intrusive_list<task> queue;
	ca::intrusive_list<task, by_age> ages;
	if (queue.first() || queue.pop() || queue.begin() != queue.end())
		return 1;
	queue.append(a);
	queue.append(c);
	queue.prepend(b);
	queue.insert(queue.iterator_to(c), d);
	if (!ids(queue, ca::array{2, 1, 4, 3}) || !static_cast<ca::list_link<task>&>(a).linked())
		return 2;
	for (task& t : queue)
		ages.append(t);
	ages.sort([](const task& left, const task& right) {
		return left.age < right.age;
	});
	if (!ids(ages, ca::array{2, 4, 3, 1}))
		return 3;
	queue.remove(a);
	if (!ids(queue, ca::array{2, 4, 3}) || static_cast<ca::list_link<task>&>(a).linked() || !ids(ages, ca::array{2, 4, 3, 1}))
		return 4;
	queue.reverse();
	if (!ids(queue, ca::array{3, 4, 2}) || queue.pop_front().value_or(a).id != 3 || queue.pop().value_or(a).id != 2)
		return 5;
	ca::intrusive_list<task> other;
	other.append(a);
	other.append(c);
	queue.splice(queue.end(), ca::move(other));
	if (!other.empty() || !ids(queue, ca::array{4, 1, 3}))
		return 6;
	const task copy = a;
	if (static_cast<const ca::list_link<task>&>(copy).linked())
		return 7;
	ca::intrusive_list<task> moved = ca::move(queue);
	if (!queue.empty() || !ids(moved, ca::array{4, 1, 3}) || moved.first().value_or(b).id != 4 || moved.last().value_or(b).id != 3)
		return 8;
	moved.sort([](const task& left, const task& right) {
		return left.id > right.id;
	});
	if (!ids(moved, ca::array{4, 3, 1}))
		return 9;
	moved.clear();
	ages.clear();
	return moved.empty() && !static_cast<ca::list_link<task>&>(c).linked() ? 0 : 10;
}
