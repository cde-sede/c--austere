#include "3-system/error/errc.hpp"

#include <errno.h>


static_assert(ca::errc::not_permitted.code() == EPERM);
static_assert(ca::errc::no_entry.code() == ENOENT);
static_assert(ca::errc::interrupted.code() == EINTR);
static_assert(ca::errc::bad_descriptor.code() == EBADF);
static_assert(ca::errc::out_of_memory.code() == ENOMEM);
static_assert(ca::errc::bad_address.code() == EFAULT);
static_assert(ca::errc::access_denied.code() == EACCES);
static_assert(ca::errc::exists.code() == EEXIST);
static_assert(ca::errc::not_a_directory.code() == ENOTDIR);
static_assert(ca::errc::is_a_directory.code() == EISDIR);
static_assert(ca::errc::invalid_argument.code() == EINVAL);
static_assert(ca::errc::no_space.code() == ENOSPC);
static_assert(ca::errc::out_of_range.code() == ERANGE);
static_assert(ca::errc::try_again.code() == EAGAIN);
static_assert(ca::errc::name_too_long.code() == ENAMETOOLONG);
static_assert(ca::errc::not_empty.code() == ENOTEMPTY);
static_assert(ca::errc::symlink_loop.code() == ELOOP);
static_assert(ca::errc::not_supported.code() == ENOTSUP);
static_assert(ca::errc::not_implemented.code() == ENOSYS);
static_assert(ca::errc::timed_out.code() == ETIMEDOUT);
static_assert(ca::errc::not_recoverable.code() == ENOTRECOVERABLE);
