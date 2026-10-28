#include "3-system/thread/impl/ordered_keys.hpp"
#include "3-system/thread/thread_key_create.hpp"


[[nodiscard]] auto ca::sys::thread_key_create(void (* const on_exit)(void*), const ca::sys::exit_stage stage) noexcept -> ca::result<ca::sys::thread_key> {
	return ca::sys::impl::create_ordered_key(on_exit, stage).transform([](const ca::usz id) { return ca::sys::thread_key{id}; });
}
