#include "3-system/fs/options.hpp"

#include <fcntl.h>


static_assert(ca::fs::access::READ_ONLY == O_RDONLY);
static_assert(ca::fs::access::WRITE_ONLY == O_WRONLY);
static_assert(ca::fs::access::READ_WRITE == O_RDWR);
static_assert(ca::fs::descriptor_flags::GET == F_GETFD);
static_assert(ca::fs::descriptor_flags::SET == F_SETFD);
static_assert(ca::fs::status_flags::GET == F_GETFL);
static_assert(ca::fs::status_flags::SET == F_SETFL);
static_assert(ca::fs::create_t::OPEN_VALUE == O_CREAT);
static_assert(ca::fs::exclusive_t::OPEN_VALUE == O_EXCL);
static_assert(ca::fs::truncate_t::OPEN_VALUE == O_TRUNC);
static_assert(ca::fs::directory_t::OPEN_VALUE == O_DIRECTORY);
static_assert(ca::fs::nofollow_t::OPEN_VALUE == O_NOFOLLOW);
static_assert(ca::fs::sync_t::OPEN_VALUE == O_SYNC);
static_assert(ca::fs::append_t::VALUE == O_APPEND && ca::fs::append_t::OPEN_VALUE == O_APPEND);
static_assert(ca::fs::nonblock_t::VALUE == O_NONBLOCK && ca::fs::nonblock_t::OPEN_VALUE == O_NONBLOCK);
static_assert(ca::fs::cloexec_t::VALUE == FD_CLOEXEC && ca::fs::cloexec_t::OPEN_VALUE == O_CLOEXEC);
#ifdef FD_CLOFORK
static_assert(ca::fs::clofork_t::SUPPORTED, "ca: the platform has FD_CLOFORK, enable clofork_t");
#else
static_assert(!ca::fs::clofork_t::SUPPORTED);
#endif
