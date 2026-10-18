#include "3-system/process/signals.hpp"

#include <signal.h>


static_assert(ca::sys::signals::hangup == SIGHUP);
static_assert(ca::sys::signals::interrupt == SIGINT);
static_assert(ca::sys::signals::quit == SIGQUIT);
static_assert(ca::sys::signals::abort == SIGABRT);
static_assert(ca::sys::signals::kill == SIGKILL);
static_assert(ca::sys::signals::segv == SIGSEGV);
static_assert(ca::sys::signals::pipe == SIGPIPE);
static_assert(ca::sys::signals::alarm == SIGALRM);
static_assert(ca::sys::signals::terminate == SIGTERM);
static_assert(ca::sys::signals::user1 == SIGUSR1);
static_assert(ca::sys::signals::user2 == SIGUSR2);
static_assert(ca::sys::signals::child == SIGCHLD);
