#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/io/poll.hpp"
#include "3-system/io/poll_events.hpp"
#include "3-system/syscall/syscalls.hpp"

#include <poll.h>


namespace ca {


	namespace sys {


		namespace impl {

			struct poll_layout final {
					using self = ca::sys::impl::poll_layout;
					static_assert(sizeof(ca::sys::poll_entry) == sizeof(::pollfd) && alignof(ca::sys::poll_entry) == alignof(::pollfd));
					static_assert(__builtin_offsetof(ca::sys::poll_entry, _fd) == __builtin_offsetof(::pollfd, fd));
					static_assert(__builtin_offsetof(ca::sys::poll_entry, _events) == __builtin_offsetof(::pollfd, events));
					static_assert(__builtin_offsetof(ca::sys::poll_entry, _reported) == __builtin_offsetof(::pollfd, revents));
			}; // struct poll_layout

		} // namespace impl

	} // namespace sys

} // namespace ca

static_assert(ca::sys::poll_events::readable == POLLIN);
static_assert(ca::sys::poll_events::urgent == POLLPRI);
static_assert(ca::sys::poll_events::writable == POLLOUT);
static_assert(ca::sys::poll_events::error == POLLERR);
static_assert(ca::sys::poll_events::hangup == POLLHUP);
static_assert(ca::sys::poll_events::invalid == POLLNVAL);

[[nodiscard]] auto ca::sys::poll(const ca::span<ca::sys::poll_entry> entries, const ca::i32 timeout_ms) noexcept -> ca::result<ca::usz> {

#if ca_platform(linux) && ca_architecture(x86_64)
	return ca::syscall::poll(entries.data(), entries.size(), timeout_ms).transform([](const ca::isz ready) {
		return static_cast<ca::usz>(ready);
	});
#else
	const int ready = ::poll(reinterpret_cast<::pollfd*>(entries.data()), static_cast<nfds_t>(entries.size()), timeout_ms);
	if (ready == -1) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return static_cast<ca::usz>(ready);
#endif
}
