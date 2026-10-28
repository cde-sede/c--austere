// expect: exit 0
// link: both
#include "3-system/thread/impl/key_table.hpp"
#include "3-system/thread/impl/thread_exit.hpp"

static char order[64];
static int ran = 0;
static char names[3] = {'a', 'b', 'c'};
static ca::usz again_key = 0U;

static auto record(void* const name) noexcept -> void {
	order[ran++] = *static_cast<char*>(name);
}

static auto register_more(void* const name) noexcept -> void {

	record(name);
	static_cast<void>(ca::sys::impl::at_thread_exit(&record, &names[2]));
}

static auto set_again(void* const name) noexcept -> void {

	record(name);
	if (ran < 5)
		static_cast<void>(ca::sys::impl::set_key(again_key, name));
}

auto main(void) -> int {
	for (int i = 0; i < 20; ++i) {
		if (!ca::sys::impl::at_thread_exit(&record, &names[0]))
			return 1;
	}
	if (!ca::sys::impl::at_thread_exit(&register_more, &names[1]))
		return 2;
	ca::sys::impl::run_thread_exit_handlers();
	if (ran != 22 || order[0] != 'b' || order[1] != 'c' || order[2] != 'a' || order[21] != 'a')
		return 3;
	ca::sys::impl::run_thread_exit_handlers();
	if (ran != 22)
		return 4;

	ran = 0;
	auto first = ca::sys::impl::create_key(&record);
	auto second = ca::sys::impl::create_key(&set_again);
	if (!first || !second || *second != *first + 1U)
		return 5;
	again_key = *second;
	if (ca::sys::impl::set_key(*second + 1U, &names[0]) || ca::sys::impl::key_value(*second + 1U) != nullptr)
		return 6;
	if (!ca::sys::impl::set_key(*first, &names[0]) || !ca::sys::impl::set_key(*second, &names[1]) || ca::sys::impl::key_value(*first) != &names[0])
		return 7;
	ca::sys::impl::run_key_destructors();
	if (ca::sys::impl::key_value(*first) != nullptr || ca::sys::impl::key_value(*second) != nullptr)
		return 8;
	return ran == 5 && order[0] == 'a' && order[1] == 'b' && order[2] == 'b' && order[4] == 'b' ? 0 : 9;
}
