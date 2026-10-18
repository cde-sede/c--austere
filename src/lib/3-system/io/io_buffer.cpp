#include "3-system/io/io_buffer.hpp"

#include <sys/uio.h>


namespace ca {


	namespace sys {


		namespace impl {

			struct io_buffer_layout final {
					using self = ca::sys::impl::io_buffer_layout;
					static_assert(sizeof(ca::sys::io_buffer) == sizeof(::iovec) && alignof(ca::sys::io_buffer) == alignof(::iovec));
					static_assert(__builtin_offsetof(ca::sys::io_buffer, _data) == __builtin_offsetof(::iovec, iov_base));
					static_assert(__builtin_offsetof(ca::sys::io_buffer, _size) == __builtin_offsetof(::iovec, iov_len));
			}; // struct io_buffer_layout

		} // namespace impl

	} // namespace sys

} // namespace ca
