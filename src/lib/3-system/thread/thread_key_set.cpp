#include "3-system/thread/impl/ordered_keys.hpp"
#include "3-system/thread/thread_key_set.hpp"


[[nodiscard]] auto ca::sys::thread_key_set(const ca::sys::thread_key key, void* const value) noexcept -> ca::result<void> {
	return ca::sys::impl::set_ordered_key(key.id(), value);
}
