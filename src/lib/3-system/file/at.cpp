#include "3-system/file/at.hpp"

#include <fcntl.h>


static_assert(ca::sys::at::cwd == AT_FDCWD);
static_assert(ca::sys::at::symlink_nofollow == AT_SYMLINK_NOFOLLOW);
static_assert(ca::sys::at::symlink_follow == AT_SYMLINK_FOLLOW);
static_assert(ca::sys::at::remove_directory == AT_REMOVEDIR);
static_assert(ca::sys::at::effective_ids == AT_EACCESS);
