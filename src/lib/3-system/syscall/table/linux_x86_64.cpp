#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/syscall/table/linux_x86_64.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)
#	include <sys/syscall.h>

#	if defined(SYS_read)
static_assert(ca::syscall::number::read == SYS_read);
#	endif
#	if defined(SYS_write)
static_assert(ca::syscall::number::write == SYS_write);
#	endif
#	if defined(SYS_open)
static_assert(ca::syscall::number::open == SYS_open);
#	endif
#	if defined(SYS_close)
static_assert(ca::syscall::number::close == SYS_close);
#	endif
#	if defined(SYS_stat)
static_assert(ca::syscall::number::stat == SYS_stat);
#	endif
#	if defined(SYS_fstat)
static_assert(ca::syscall::number::fstat == SYS_fstat);
#	endif
#	if defined(SYS_lstat)
static_assert(ca::syscall::number::lstat == SYS_lstat);
#	endif
#	if defined(SYS_poll)
static_assert(ca::syscall::number::poll == SYS_poll);
#	endif
#	if defined(SYS_lseek)
static_assert(ca::syscall::number::lseek == SYS_lseek);
#	endif
#	if defined(SYS_mmap)
static_assert(ca::syscall::number::mmap == SYS_mmap);
#	endif
#	if defined(SYS_mprotect)
static_assert(ca::syscall::number::mprotect == SYS_mprotect);
#	endif
#	if defined(SYS_munmap)
static_assert(ca::syscall::number::munmap == SYS_munmap);
#	endif
#	if defined(SYS_brk)
static_assert(ca::syscall::number::brk == SYS_brk);
#	endif
#	if defined(SYS_rt_sigaction)
static_assert(ca::syscall::number::rt_sigaction == SYS_rt_sigaction);
#	endif
#	if defined(SYS_rt_sigprocmask)
static_assert(ca::syscall::number::rt_sigprocmask == SYS_rt_sigprocmask);
#	endif
#	if defined(SYS_rt_sigreturn)
static_assert(ca::syscall::number::rt_sigreturn == SYS_rt_sigreturn);
#	endif
#	if defined(SYS_ioctl)
static_assert(ca::syscall::number::ioctl == SYS_ioctl);
#	endif
#	if defined(SYS_pread64)
static_assert(ca::syscall::number::pread64 == SYS_pread64);
#	endif
#	if defined(SYS_pwrite64)
static_assert(ca::syscall::number::pwrite64 == SYS_pwrite64);
#	endif
#	if defined(SYS_readv)
static_assert(ca::syscall::number::readv == SYS_readv);
#	endif
#	if defined(SYS_writev)
static_assert(ca::syscall::number::writev == SYS_writev);
#	endif
#	if defined(SYS_access)
static_assert(ca::syscall::number::access == SYS_access);
#	endif
#	if defined(SYS_pipe)
static_assert(ca::syscall::number::pipe == SYS_pipe);
#	endif
#	if defined(SYS_select)
static_assert(ca::syscall::number::select == SYS_select);
#	endif
#	if defined(SYS_sched_yield)
static_assert(ca::syscall::number::sched_yield == SYS_sched_yield);
#	endif
#	if defined(SYS_mremap)
static_assert(ca::syscall::number::mremap == SYS_mremap);
#	endif
#	if defined(SYS_msync)
static_assert(ca::syscall::number::msync == SYS_msync);
#	endif
#	if defined(SYS_mincore)
static_assert(ca::syscall::number::mincore == SYS_mincore);
#	endif
#	if defined(SYS_madvise)
static_assert(ca::syscall::number::madvise == SYS_madvise);
#	endif
#	if defined(SYS_shmget)
static_assert(ca::syscall::number::shmget == SYS_shmget);
#	endif
#	if defined(SYS_shmat)
static_assert(ca::syscall::number::shmat == SYS_shmat);
#	endif
#	if defined(SYS_shmctl)
static_assert(ca::syscall::number::shmctl == SYS_shmctl);
#	endif
#	if defined(SYS_dup)
static_assert(ca::syscall::number::dup == SYS_dup);
#	endif
#	if defined(SYS_dup2)
static_assert(ca::syscall::number::dup2 == SYS_dup2);
#	endif
#	if defined(SYS_pause)
static_assert(ca::syscall::number::pause == SYS_pause);
#	endif
#	if defined(SYS_nanosleep)
static_assert(ca::syscall::number::nanosleep == SYS_nanosleep);
#	endif
#	if defined(SYS_getitimer)
static_assert(ca::syscall::number::getitimer == SYS_getitimer);
#	endif
#	if defined(SYS_alarm)
static_assert(ca::syscall::number::alarm == SYS_alarm);
#	endif
#	if defined(SYS_setitimer)
static_assert(ca::syscall::number::setitimer == SYS_setitimer);
#	endif
#	if defined(SYS_getpid)
static_assert(ca::syscall::number::getpid == SYS_getpid);
#	endif
#	if defined(SYS_sendfile)
static_assert(ca::syscall::number::sendfile == SYS_sendfile);
#	endif
#	if defined(SYS_socket)
static_assert(ca::syscall::number::socket == SYS_socket);
#	endif
#	if defined(SYS_connect)
static_assert(ca::syscall::number::connect == SYS_connect);
#	endif
#	if defined(SYS_accept)
static_assert(ca::syscall::number::accept == SYS_accept);
#	endif
#	if defined(SYS_sendto)
static_assert(ca::syscall::number::sendto == SYS_sendto);
#	endif
#	if defined(SYS_recvfrom)
static_assert(ca::syscall::number::recvfrom == SYS_recvfrom);
#	endif
#	if defined(SYS_sendmsg)
static_assert(ca::syscall::number::sendmsg == SYS_sendmsg);
#	endif
#	if defined(SYS_recvmsg)
static_assert(ca::syscall::number::recvmsg == SYS_recvmsg);
#	endif
#	if defined(SYS_shutdown)
static_assert(ca::syscall::number::shutdown == SYS_shutdown);
#	endif
#	if defined(SYS_bind)
static_assert(ca::syscall::number::bind == SYS_bind);
#	endif
#	if defined(SYS_listen)
static_assert(ca::syscall::number::listen == SYS_listen);
#	endif
#	if defined(SYS_getsockname)
static_assert(ca::syscall::number::getsockname == SYS_getsockname);
#	endif
#	if defined(SYS_getpeername)
static_assert(ca::syscall::number::getpeername == SYS_getpeername);
#	endif
#	if defined(SYS_socketpair)
static_assert(ca::syscall::number::socketpair == SYS_socketpair);
#	endif
#	if defined(SYS_setsockopt)
static_assert(ca::syscall::number::setsockopt == SYS_setsockopt);
#	endif
#	if defined(SYS_getsockopt)
static_assert(ca::syscall::number::getsockopt == SYS_getsockopt);
#	endif
#	if defined(SYS_clone)
static_assert(ca::syscall::number::clone == SYS_clone);
#	endif
#	if defined(SYS_fork)
static_assert(ca::syscall::number::fork == SYS_fork);
#	endif
#	if defined(SYS_vfork)
static_assert(ca::syscall::number::vfork == SYS_vfork);
#	endif
#	if defined(SYS_execve)
static_assert(ca::syscall::number::execve == SYS_execve);
#	endif
#	if defined(SYS_exit)
static_assert(ca::syscall::number::exit == SYS_exit);
#	endif
#	if defined(SYS_wait4)
static_assert(ca::syscall::number::wait4 == SYS_wait4);
#	endif
#	if defined(SYS_kill)
static_assert(ca::syscall::number::kill == SYS_kill);
#	endif
#	if defined(SYS_uname)
static_assert(ca::syscall::number::uname == SYS_uname);
#	endif
#	if defined(SYS_semget)
static_assert(ca::syscall::number::semget == SYS_semget);
#	endif
#	if defined(SYS_semop)
static_assert(ca::syscall::number::semop == SYS_semop);
#	endif
#	if defined(SYS_semctl)
static_assert(ca::syscall::number::semctl == SYS_semctl);
#	endif
#	if defined(SYS_shmdt)
static_assert(ca::syscall::number::shmdt == SYS_shmdt);
#	endif
#	if defined(SYS_msgget)
static_assert(ca::syscall::number::msgget == SYS_msgget);
#	endif
#	if defined(SYS_msgsnd)
static_assert(ca::syscall::number::msgsnd == SYS_msgsnd);
#	endif
#	if defined(SYS_msgrcv)
static_assert(ca::syscall::number::msgrcv == SYS_msgrcv);
#	endif
#	if defined(SYS_msgctl)
static_assert(ca::syscall::number::msgctl == SYS_msgctl);
#	endif
#	if defined(SYS_fcntl)
static_assert(ca::syscall::number::fcntl == SYS_fcntl);
#	endif
#	if defined(SYS_flock)
static_assert(ca::syscall::number::flock == SYS_flock);
#	endif
#	if defined(SYS_fsync)
static_assert(ca::syscall::number::fsync == SYS_fsync);
#	endif
#	if defined(SYS_fdatasync)
static_assert(ca::syscall::number::fdatasync == SYS_fdatasync);
#	endif
#	if defined(SYS_truncate)
static_assert(ca::syscall::number::truncate == SYS_truncate);
#	endif
#	if defined(SYS_ftruncate)
static_assert(ca::syscall::number::ftruncate == SYS_ftruncate);
#	endif
#	if defined(SYS_getdents)
static_assert(ca::syscall::number::getdents == SYS_getdents);
#	endif
#	if defined(SYS_getcwd)
static_assert(ca::syscall::number::getcwd == SYS_getcwd);
#	endif
#	if defined(SYS_chdir)
static_assert(ca::syscall::number::chdir == SYS_chdir);
#	endif
#	if defined(SYS_fchdir)
static_assert(ca::syscall::number::fchdir == SYS_fchdir);
#	endif
#	if defined(SYS_rename)
static_assert(ca::syscall::number::rename == SYS_rename);
#	endif
#	if defined(SYS_mkdir)
static_assert(ca::syscall::number::mkdir == SYS_mkdir);
#	endif
#	if defined(SYS_rmdir)
static_assert(ca::syscall::number::rmdir == SYS_rmdir);
#	endif
#	if defined(SYS_creat)
static_assert(ca::syscall::number::creat == SYS_creat);
#	endif
#	if defined(SYS_link)
static_assert(ca::syscall::number::link == SYS_link);
#	endif
#	if defined(SYS_unlink)
static_assert(ca::syscall::number::unlink == SYS_unlink);
#	endif
#	if defined(SYS_symlink)
static_assert(ca::syscall::number::symlink == SYS_symlink);
#	endif
#	if defined(SYS_readlink)
static_assert(ca::syscall::number::readlink == SYS_readlink);
#	endif
#	if defined(SYS_chmod)
static_assert(ca::syscall::number::chmod == SYS_chmod);
#	endif
#	if defined(SYS_fchmod)
static_assert(ca::syscall::number::fchmod == SYS_fchmod);
#	endif
#	if defined(SYS_chown)
static_assert(ca::syscall::number::chown == SYS_chown);
#	endif
#	if defined(SYS_fchown)
static_assert(ca::syscall::number::fchown == SYS_fchown);
#	endif
#	if defined(SYS_lchown)
static_assert(ca::syscall::number::lchown == SYS_lchown);
#	endif
#	if defined(SYS_umask)
static_assert(ca::syscall::number::umask == SYS_umask);
#	endif
#	if defined(SYS_gettimeofday)
static_assert(ca::syscall::number::gettimeofday == SYS_gettimeofday);
#	endif
#	if defined(SYS_getrlimit)
static_assert(ca::syscall::number::getrlimit == SYS_getrlimit);
#	endif
#	if defined(SYS_getrusage)
static_assert(ca::syscall::number::getrusage == SYS_getrusage);
#	endif
#	if defined(SYS_sysinfo)
static_assert(ca::syscall::number::sysinfo == SYS_sysinfo);
#	endif
#	if defined(SYS_times)
static_assert(ca::syscall::number::times == SYS_times);
#	endif
#	if defined(SYS_ptrace)
static_assert(ca::syscall::number::ptrace == SYS_ptrace);
#	endif
#	if defined(SYS_getuid)
static_assert(ca::syscall::number::getuid == SYS_getuid);
#	endif
#	if defined(SYS_syslog)
static_assert(ca::syscall::number::syslog == SYS_syslog);
#	endif
#	if defined(SYS_getgid)
static_assert(ca::syscall::number::getgid == SYS_getgid);
#	endif
#	if defined(SYS_setuid)
static_assert(ca::syscall::number::setuid == SYS_setuid);
#	endif
#	if defined(SYS_setgid)
static_assert(ca::syscall::number::setgid == SYS_setgid);
#	endif
#	if defined(SYS_geteuid)
static_assert(ca::syscall::number::geteuid == SYS_geteuid);
#	endif
#	if defined(SYS_getegid)
static_assert(ca::syscall::number::getegid == SYS_getegid);
#	endif
#	if defined(SYS_setpgid)
static_assert(ca::syscall::number::setpgid == SYS_setpgid);
#	endif
#	if defined(SYS_getppid)
static_assert(ca::syscall::number::getppid == SYS_getppid);
#	endif
#	if defined(SYS_getpgrp)
static_assert(ca::syscall::number::getpgrp == SYS_getpgrp);
#	endif
#	if defined(SYS_setsid)
static_assert(ca::syscall::number::setsid == SYS_setsid);
#	endif
#	if defined(SYS_setreuid)
static_assert(ca::syscall::number::setreuid == SYS_setreuid);
#	endif
#	if defined(SYS_setregid)
static_assert(ca::syscall::number::setregid == SYS_setregid);
#	endif
#	if defined(SYS_getgroups)
static_assert(ca::syscall::number::getgroups == SYS_getgroups);
#	endif
#	if defined(SYS_setgroups)
static_assert(ca::syscall::number::setgroups == SYS_setgroups);
#	endif
#	if defined(SYS_setresuid)
static_assert(ca::syscall::number::setresuid == SYS_setresuid);
#	endif
#	if defined(SYS_getresuid)
static_assert(ca::syscall::number::getresuid == SYS_getresuid);
#	endif
#	if defined(SYS_setresgid)
static_assert(ca::syscall::number::setresgid == SYS_setresgid);
#	endif
#	if defined(SYS_getresgid)
static_assert(ca::syscall::number::getresgid == SYS_getresgid);
#	endif
#	if defined(SYS_getpgid)
static_assert(ca::syscall::number::getpgid == SYS_getpgid);
#	endif
#	if defined(SYS_setfsuid)
static_assert(ca::syscall::number::setfsuid == SYS_setfsuid);
#	endif
#	if defined(SYS_setfsgid)
static_assert(ca::syscall::number::setfsgid == SYS_setfsgid);
#	endif
#	if defined(SYS_getsid)
static_assert(ca::syscall::number::getsid == SYS_getsid);
#	endif
#	if defined(SYS_capget)
static_assert(ca::syscall::number::capget == SYS_capget);
#	endif
#	if defined(SYS_capset)
static_assert(ca::syscall::number::capset == SYS_capset);
#	endif
#	if defined(SYS_rt_sigpending)
static_assert(ca::syscall::number::rt_sigpending == SYS_rt_sigpending);
#	endif
#	if defined(SYS_rt_sigtimedwait)
static_assert(ca::syscall::number::rt_sigtimedwait == SYS_rt_sigtimedwait);
#	endif
#	if defined(SYS_rt_sigqueueinfo)
static_assert(ca::syscall::number::rt_sigqueueinfo == SYS_rt_sigqueueinfo);
#	endif
#	if defined(SYS_rt_sigsuspend)
static_assert(ca::syscall::number::rt_sigsuspend == SYS_rt_sigsuspend);
#	endif
#	if defined(SYS_sigaltstack)
static_assert(ca::syscall::number::sigaltstack == SYS_sigaltstack);
#	endif
#	if defined(SYS_utime)
static_assert(ca::syscall::number::utime == SYS_utime);
#	endif
#	if defined(SYS_mknod)
static_assert(ca::syscall::number::mknod == SYS_mknod);
#	endif
#	if defined(SYS_personality)
static_assert(ca::syscall::number::personality == SYS_personality);
#	endif
#	if defined(SYS_ustat)
static_assert(ca::syscall::number::ustat == SYS_ustat);
#	endif
#	if defined(SYS_statfs)
static_assert(ca::syscall::number::statfs == SYS_statfs);
#	endif
#	if defined(SYS_fstatfs)
static_assert(ca::syscall::number::fstatfs == SYS_fstatfs);
#	endif
#	if defined(SYS_sysfs)
static_assert(ca::syscall::number::sysfs == SYS_sysfs);
#	endif
#	if defined(SYS_getpriority)
static_assert(ca::syscall::number::getpriority == SYS_getpriority);
#	endif
#	if defined(SYS_setpriority)
static_assert(ca::syscall::number::setpriority == SYS_setpriority);
#	endif
#	if defined(SYS_sched_setparam)
static_assert(ca::syscall::number::sched_setparam == SYS_sched_setparam);
#	endif
#	if defined(SYS_sched_getparam)
static_assert(ca::syscall::number::sched_getparam == SYS_sched_getparam);
#	endif
#	if defined(SYS_sched_setscheduler)
static_assert(ca::syscall::number::sched_setscheduler == SYS_sched_setscheduler);
#	endif
#	if defined(SYS_sched_getscheduler)
static_assert(ca::syscall::number::sched_getscheduler == SYS_sched_getscheduler);
#	endif
#	if defined(SYS_sched_get_priority_max)
static_assert(ca::syscall::number::sched_get_priority_max == SYS_sched_get_priority_max);
#	endif
#	if defined(SYS_sched_get_priority_min)
static_assert(ca::syscall::number::sched_get_priority_min == SYS_sched_get_priority_min);
#	endif
#	if defined(SYS_sched_rr_get_interval)
static_assert(ca::syscall::number::sched_rr_get_interval == SYS_sched_rr_get_interval);
#	endif
#	if defined(SYS_mlock)
static_assert(ca::syscall::number::mlock == SYS_mlock);
#	endif
#	if defined(SYS_munlock)
static_assert(ca::syscall::number::munlock == SYS_munlock);
#	endif
#	if defined(SYS_mlockall)
static_assert(ca::syscall::number::mlockall == SYS_mlockall);
#	endif
#	if defined(SYS_munlockall)
static_assert(ca::syscall::number::munlockall == SYS_munlockall);
#	endif
#	if defined(SYS_vhangup)
static_assert(ca::syscall::number::vhangup == SYS_vhangup);
#	endif
#	if defined(SYS_modify_ldt)
static_assert(ca::syscall::number::modify_ldt == SYS_modify_ldt);
#	endif
#	if defined(SYS_pivot_root)
static_assert(ca::syscall::number::pivot_root == SYS_pivot_root);
#	endif
#	if defined(SYS_prctl)
static_assert(ca::syscall::number::prctl == SYS_prctl);
#	endif
#	if defined(SYS_arch_prctl)
static_assert(ca::syscall::number::arch_prctl == SYS_arch_prctl);
#	endif
#	if defined(SYS_adjtimex)
static_assert(ca::syscall::number::adjtimex == SYS_adjtimex);
#	endif
#	if defined(SYS_setrlimit)
static_assert(ca::syscall::number::setrlimit == SYS_setrlimit);
#	endif
#	if defined(SYS_chroot)
static_assert(ca::syscall::number::chroot == SYS_chroot);
#	endif
#	if defined(SYS_sync)
static_assert(ca::syscall::number::sync == SYS_sync);
#	endif
#	if defined(SYS_acct)
static_assert(ca::syscall::number::acct == SYS_acct);
#	endif
#	if defined(SYS_settimeofday)
static_assert(ca::syscall::number::settimeofday == SYS_settimeofday);
#	endif
#	if defined(SYS_mount)
static_assert(ca::syscall::number::mount == SYS_mount);
#	endif
#	if defined(SYS_umount2)
static_assert(ca::syscall::number::umount2 == SYS_umount2);
#	endif
#	if defined(SYS_swapon)
static_assert(ca::syscall::number::swapon == SYS_swapon);
#	endif
#	if defined(SYS_swapoff)
static_assert(ca::syscall::number::swapoff == SYS_swapoff);
#	endif
#	if defined(SYS_reboot)
static_assert(ca::syscall::number::reboot == SYS_reboot);
#	endif
#	if defined(SYS_sethostname)
static_assert(ca::syscall::number::sethostname == SYS_sethostname);
#	endif
#	if defined(SYS_setdomainname)
static_assert(ca::syscall::number::setdomainname == SYS_setdomainname);
#	endif
#	if defined(SYS_iopl)
static_assert(ca::syscall::number::iopl == SYS_iopl);
#	endif
#	if defined(SYS_ioperm)
static_assert(ca::syscall::number::ioperm == SYS_ioperm);
#	endif
#	if defined(SYS_init_module)
static_assert(ca::syscall::number::init_module == SYS_init_module);
#	endif
#	if defined(SYS_delete_module)
static_assert(ca::syscall::number::delete_module == SYS_delete_module);
#	endif
#	if defined(SYS_quotactl)
static_assert(ca::syscall::number::quotactl == SYS_quotactl);
#	endif
#	if defined(SYS_gettid)
static_assert(ca::syscall::number::gettid == SYS_gettid);
#	endif
#	if defined(SYS_readahead)
static_assert(ca::syscall::number::readahead == SYS_readahead);
#	endif
#	if defined(SYS_setxattr)
static_assert(ca::syscall::number::setxattr == SYS_setxattr);
#	endif
#	if defined(SYS_lsetxattr)
static_assert(ca::syscall::number::lsetxattr == SYS_lsetxattr);
#	endif
#	if defined(SYS_fsetxattr)
static_assert(ca::syscall::number::fsetxattr == SYS_fsetxattr);
#	endif
#	if defined(SYS_getxattr)
static_assert(ca::syscall::number::getxattr == SYS_getxattr);
#	endif
#	if defined(SYS_lgetxattr)
static_assert(ca::syscall::number::lgetxattr == SYS_lgetxattr);
#	endif
#	if defined(SYS_fgetxattr)
static_assert(ca::syscall::number::fgetxattr == SYS_fgetxattr);
#	endif
#	if defined(SYS_listxattr)
static_assert(ca::syscall::number::listxattr == SYS_listxattr);
#	endif
#	if defined(SYS_llistxattr)
static_assert(ca::syscall::number::llistxattr == SYS_llistxattr);
#	endif
#	if defined(SYS_flistxattr)
static_assert(ca::syscall::number::flistxattr == SYS_flistxattr);
#	endif
#	if defined(SYS_removexattr)
static_assert(ca::syscall::number::removexattr == SYS_removexattr);
#	endif
#	if defined(SYS_lremovexattr)
static_assert(ca::syscall::number::lremovexattr == SYS_lremovexattr);
#	endif
#	if defined(SYS_fremovexattr)
static_assert(ca::syscall::number::fremovexattr == SYS_fremovexattr);
#	endif
#	if defined(SYS_tkill)
static_assert(ca::syscall::number::tkill == SYS_tkill);
#	endif
#	if defined(SYS_time)
static_assert(ca::syscall::number::time == SYS_time);
#	endif
#	if defined(SYS_futex)
static_assert(ca::syscall::number::futex == SYS_futex);
#	endif
#	if defined(SYS_sched_setaffinity)
static_assert(ca::syscall::number::sched_setaffinity == SYS_sched_setaffinity);
#	endif
#	if defined(SYS_sched_getaffinity)
static_assert(ca::syscall::number::sched_getaffinity == SYS_sched_getaffinity);
#	endif
#	if defined(SYS_io_setup)
static_assert(ca::syscall::number::io_setup == SYS_io_setup);
#	endif
#	if defined(SYS_io_destroy)
static_assert(ca::syscall::number::io_destroy == SYS_io_destroy);
#	endif
#	if defined(SYS_io_getevents)
static_assert(ca::syscall::number::io_getevents == SYS_io_getevents);
#	endif
#	if defined(SYS_io_submit)
static_assert(ca::syscall::number::io_submit == SYS_io_submit);
#	endif
#	if defined(SYS_io_cancel)
static_assert(ca::syscall::number::io_cancel == SYS_io_cancel);
#	endif
#	if defined(SYS_epoll_create)
static_assert(ca::syscall::number::epoll_create == SYS_epoll_create);
#	endif
#	if defined(SYS_remap_file_pages)
static_assert(ca::syscall::number::remap_file_pages == SYS_remap_file_pages);
#	endif
#	if defined(SYS_getdents64)
static_assert(ca::syscall::number::getdents64 == SYS_getdents64);
#	endif
#	if defined(SYS_set_tid_address)
static_assert(ca::syscall::number::set_tid_address == SYS_set_tid_address);
#	endif
#	if defined(SYS_restart_syscall)
static_assert(ca::syscall::number::restart_syscall == SYS_restart_syscall);
#	endif
#	if defined(SYS_semtimedop)
static_assert(ca::syscall::number::semtimedop == SYS_semtimedop);
#	endif
#	if defined(SYS_fadvise64)
static_assert(ca::syscall::number::fadvise64 == SYS_fadvise64);
#	endif
#	if defined(SYS_timer_create)
static_assert(ca::syscall::number::timer_create == SYS_timer_create);
#	endif
#	if defined(SYS_timer_settime)
static_assert(ca::syscall::number::timer_settime == SYS_timer_settime);
#	endif
#	if defined(SYS_timer_gettime)
static_assert(ca::syscall::number::timer_gettime == SYS_timer_gettime);
#	endif
#	if defined(SYS_timer_getoverrun)
static_assert(ca::syscall::number::timer_getoverrun == SYS_timer_getoverrun);
#	endif
#	if defined(SYS_timer_delete)
static_assert(ca::syscall::number::timer_delete == SYS_timer_delete);
#	endif
#	if defined(SYS_clock_settime)
static_assert(ca::syscall::number::clock_settime == SYS_clock_settime);
#	endif
#	if defined(SYS_clock_gettime)
static_assert(ca::syscall::number::clock_gettime == SYS_clock_gettime);
#	endif
#	if defined(SYS_clock_getres)
static_assert(ca::syscall::number::clock_getres == SYS_clock_getres);
#	endif
#	if defined(SYS_clock_nanosleep)
static_assert(ca::syscall::number::clock_nanosleep == SYS_clock_nanosleep);
#	endif
#	if defined(SYS_exit_group)
static_assert(ca::syscall::number::exit_group == SYS_exit_group);
#	endif
#	if defined(SYS_epoll_wait)
static_assert(ca::syscall::number::epoll_wait == SYS_epoll_wait);
#	endif
#	if defined(SYS_epoll_ctl)
static_assert(ca::syscall::number::epoll_ctl == SYS_epoll_ctl);
#	endif
#	if defined(SYS_tgkill)
static_assert(ca::syscall::number::tgkill == SYS_tgkill);
#	endif
#	if defined(SYS_utimes)
static_assert(ca::syscall::number::utimes == SYS_utimes);
#	endif
#	if defined(SYS_mbind)
static_assert(ca::syscall::number::mbind == SYS_mbind);
#	endif
#	if defined(SYS_set_mempolicy)
static_assert(ca::syscall::number::set_mempolicy == SYS_set_mempolicy);
#	endif
#	if defined(SYS_get_mempolicy)
static_assert(ca::syscall::number::get_mempolicy == SYS_get_mempolicy);
#	endif
#	if defined(SYS_mq_open)
static_assert(ca::syscall::number::mq_open == SYS_mq_open);
#	endif
#	if defined(SYS_mq_unlink)
static_assert(ca::syscall::number::mq_unlink == SYS_mq_unlink);
#	endif
#	if defined(SYS_mq_timedsend)
static_assert(ca::syscall::number::mq_timedsend == SYS_mq_timedsend);
#	endif
#	if defined(SYS_mq_timedreceive)
static_assert(ca::syscall::number::mq_timedreceive == SYS_mq_timedreceive);
#	endif
#	if defined(SYS_mq_notify)
static_assert(ca::syscall::number::mq_notify == SYS_mq_notify);
#	endif
#	if defined(SYS_mq_getsetattr)
static_assert(ca::syscall::number::mq_getsetattr == SYS_mq_getsetattr);
#	endif
#	if defined(SYS_kexec_load)
static_assert(ca::syscall::number::kexec_load == SYS_kexec_load);
#	endif
#	if defined(SYS_waitid)
static_assert(ca::syscall::number::waitid == SYS_waitid);
#	endif
#	if defined(SYS_add_key)
static_assert(ca::syscall::number::add_key == SYS_add_key);
#	endif
#	if defined(SYS_request_key)
static_assert(ca::syscall::number::request_key == SYS_request_key);
#	endif
#	if defined(SYS_keyctl)
static_assert(ca::syscall::number::keyctl == SYS_keyctl);
#	endif
#	if defined(SYS_ioprio_set)
static_assert(ca::syscall::number::ioprio_set == SYS_ioprio_set);
#	endif
#	if defined(SYS_ioprio_get)
static_assert(ca::syscall::number::ioprio_get == SYS_ioprio_get);
#	endif
#	if defined(SYS_inotify_init)
static_assert(ca::syscall::number::inotify_init == SYS_inotify_init);
#	endif
#	if defined(SYS_inotify_add_watch)
static_assert(ca::syscall::number::inotify_add_watch == SYS_inotify_add_watch);
#	endif
#	if defined(SYS_inotify_rm_watch)
static_assert(ca::syscall::number::inotify_rm_watch == SYS_inotify_rm_watch);
#	endif
#	if defined(SYS_migrate_pages)
static_assert(ca::syscall::number::migrate_pages == SYS_migrate_pages);
#	endif
#	if defined(SYS_openat)
static_assert(ca::syscall::number::openat == SYS_openat);
#	endif
#	if defined(SYS_mkdirat)
static_assert(ca::syscall::number::mkdirat == SYS_mkdirat);
#	endif
#	if defined(SYS_mknodat)
static_assert(ca::syscall::number::mknodat == SYS_mknodat);
#	endif
#	if defined(SYS_fchownat)
static_assert(ca::syscall::number::fchownat == SYS_fchownat);
#	endif
#	if defined(SYS_futimesat)
static_assert(ca::syscall::number::futimesat == SYS_futimesat);
#	endif
#	if defined(SYS_newfstatat)
static_assert(ca::syscall::number::newfstatat == SYS_newfstatat);
#	endif
#	if defined(SYS_unlinkat)
static_assert(ca::syscall::number::unlinkat == SYS_unlinkat);
#	endif
#	if defined(SYS_renameat)
static_assert(ca::syscall::number::renameat == SYS_renameat);
#	endif
#	if defined(SYS_linkat)
static_assert(ca::syscall::number::linkat == SYS_linkat);
#	endif
#	if defined(SYS_symlinkat)
static_assert(ca::syscall::number::symlinkat == SYS_symlinkat);
#	endif
#	if defined(SYS_readlinkat)
static_assert(ca::syscall::number::readlinkat == SYS_readlinkat);
#	endif
#	if defined(SYS_fchmodat)
static_assert(ca::syscall::number::fchmodat == SYS_fchmodat);
#	endif
#	if defined(SYS_faccessat)
static_assert(ca::syscall::number::faccessat == SYS_faccessat);
#	endif
#	if defined(SYS_pselect6)
static_assert(ca::syscall::number::pselect6 == SYS_pselect6);
#	endif
#	if defined(SYS_ppoll)
static_assert(ca::syscall::number::ppoll == SYS_ppoll);
#	endif
#	if defined(SYS_unshare)
static_assert(ca::syscall::number::unshare == SYS_unshare);
#	endif
#	if defined(SYS_set_robust_list)
static_assert(ca::syscall::number::set_robust_list == SYS_set_robust_list);
#	endif
#	if defined(SYS_get_robust_list)
static_assert(ca::syscall::number::get_robust_list == SYS_get_robust_list);
#	endif
#	if defined(SYS_splice)
static_assert(ca::syscall::number::splice == SYS_splice);
#	endif
#	if defined(SYS_tee)
static_assert(ca::syscall::number::tee == SYS_tee);
#	endif
#	if defined(SYS_sync_file_range)
static_assert(ca::syscall::number::sync_file_range == SYS_sync_file_range);
#	endif
#	if defined(SYS_vmsplice)
static_assert(ca::syscall::number::vmsplice == SYS_vmsplice);
#	endif
#	if defined(SYS_move_pages)
static_assert(ca::syscall::number::move_pages == SYS_move_pages);
#	endif
#	if defined(SYS_utimensat)
static_assert(ca::syscall::number::utimensat == SYS_utimensat);
#	endif
#	if defined(SYS_epoll_pwait)
static_assert(ca::syscall::number::epoll_pwait == SYS_epoll_pwait);
#	endif
#	if defined(SYS_signalfd)
static_assert(ca::syscall::number::signalfd == SYS_signalfd);
#	endif
#	if defined(SYS_timerfd_create)
static_assert(ca::syscall::number::timerfd_create == SYS_timerfd_create);
#	endif
#	if defined(SYS_eventfd)
static_assert(ca::syscall::number::eventfd == SYS_eventfd);
#	endif
#	if defined(SYS_fallocate)
static_assert(ca::syscall::number::fallocate == SYS_fallocate);
#	endif
#	if defined(SYS_timerfd_settime)
static_assert(ca::syscall::number::timerfd_settime == SYS_timerfd_settime);
#	endif
#	if defined(SYS_timerfd_gettime)
static_assert(ca::syscall::number::timerfd_gettime == SYS_timerfd_gettime);
#	endif
#	if defined(SYS_accept4)
static_assert(ca::syscall::number::accept4 == SYS_accept4);
#	endif
#	if defined(SYS_signalfd4)
static_assert(ca::syscall::number::signalfd4 == SYS_signalfd4);
#	endif
#	if defined(SYS_eventfd2)
static_assert(ca::syscall::number::eventfd2 == SYS_eventfd2);
#	endif
#	if defined(SYS_epoll_create1)
static_assert(ca::syscall::number::epoll_create1 == SYS_epoll_create1);
#	endif
#	if defined(SYS_dup3)
static_assert(ca::syscall::number::dup3 == SYS_dup3);
#	endif
#	if defined(SYS_pipe2)
static_assert(ca::syscall::number::pipe2 == SYS_pipe2);
#	endif
#	if defined(SYS_inotify_init1)
static_assert(ca::syscall::number::inotify_init1 == SYS_inotify_init1);
#	endif
#	if defined(SYS_preadv)
static_assert(ca::syscall::number::preadv == SYS_preadv);
#	endif
#	if defined(SYS_pwritev)
static_assert(ca::syscall::number::pwritev == SYS_pwritev);
#	endif
#	if defined(SYS_rt_tgsigqueueinfo)
static_assert(ca::syscall::number::rt_tgsigqueueinfo == SYS_rt_tgsigqueueinfo);
#	endif
#	if defined(SYS_perf_event_open)
static_assert(ca::syscall::number::perf_event_open == SYS_perf_event_open);
#	endif
#	if defined(SYS_recvmmsg)
static_assert(ca::syscall::number::recvmmsg == SYS_recvmmsg);
#	endif
#	if defined(SYS_fanotify_init)
static_assert(ca::syscall::number::fanotify_init == SYS_fanotify_init);
#	endif
#	if defined(SYS_fanotify_mark)
static_assert(ca::syscall::number::fanotify_mark == SYS_fanotify_mark);
#	endif
#	if defined(SYS_prlimit64)
static_assert(ca::syscall::number::prlimit64 == SYS_prlimit64);
#	endif
#	if defined(SYS_name_to_handle_at)
static_assert(ca::syscall::number::name_to_handle_at == SYS_name_to_handle_at);
#	endif
#	if defined(SYS_open_by_handle_at)
static_assert(ca::syscall::number::open_by_handle_at == SYS_open_by_handle_at);
#	endif
#	if defined(SYS_clock_adjtime)
static_assert(ca::syscall::number::clock_adjtime == SYS_clock_adjtime);
#	endif
#	if defined(SYS_syncfs)
static_assert(ca::syscall::number::syncfs == SYS_syncfs);
#	endif
#	if defined(SYS_sendmmsg)
static_assert(ca::syscall::number::sendmmsg == SYS_sendmmsg);
#	endif
#	if defined(SYS_setns)
static_assert(ca::syscall::number::setns == SYS_setns);
#	endif
#	if defined(SYS_getcpu)
static_assert(ca::syscall::number::getcpu == SYS_getcpu);
#	endif
#	if defined(SYS_process_vm_readv)
static_assert(ca::syscall::number::process_vm_readv == SYS_process_vm_readv);
#	endif
#	if defined(SYS_process_vm_writev)
static_assert(ca::syscall::number::process_vm_writev == SYS_process_vm_writev);
#	endif
#	if defined(SYS_kcmp)
static_assert(ca::syscall::number::kcmp == SYS_kcmp);
#	endif
#	if defined(SYS_finit_module)
static_assert(ca::syscall::number::finit_module == SYS_finit_module);
#	endif
#	if defined(SYS_sched_setattr)
static_assert(ca::syscall::number::sched_setattr == SYS_sched_setattr);
#	endif
#	if defined(SYS_sched_getattr)
static_assert(ca::syscall::number::sched_getattr == SYS_sched_getattr);
#	endif
#	if defined(SYS_renameat2)
static_assert(ca::syscall::number::renameat2 == SYS_renameat2);
#	endif
#	if defined(SYS_seccomp)
static_assert(ca::syscall::number::seccomp == SYS_seccomp);
#	endif
#	if defined(SYS_getrandom)
static_assert(ca::syscall::number::getrandom == SYS_getrandom);
#	endif
#	if defined(SYS_memfd_create)
static_assert(ca::syscall::number::memfd_create == SYS_memfd_create);
#	endif
#	if defined(SYS_kexec_file_load)
static_assert(ca::syscall::number::kexec_file_load == SYS_kexec_file_load);
#	endif
#	if defined(SYS_bpf)
static_assert(ca::syscall::number::bpf == SYS_bpf);
#	endif
#	if defined(SYS_execveat)
static_assert(ca::syscall::number::execveat == SYS_execveat);
#	endif
#	if defined(SYS_userfaultfd)
static_assert(ca::syscall::number::userfaultfd == SYS_userfaultfd);
#	endif
#	if defined(SYS_membarrier)
static_assert(ca::syscall::number::membarrier == SYS_membarrier);
#	endif
#	if defined(SYS_mlock2)
static_assert(ca::syscall::number::mlock2 == SYS_mlock2);
#	endif
#	if defined(SYS_copy_file_range)
static_assert(ca::syscall::number::copy_file_range == SYS_copy_file_range);
#	endif
#	if defined(SYS_preadv2)
static_assert(ca::syscall::number::preadv2 == SYS_preadv2);
#	endif
#	if defined(SYS_pwritev2)
static_assert(ca::syscall::number::pwritev2 == SYS_pwritev2);
#	endif
#	if defined(SYS_pkey_mprotect)
static_assert(ca::syscall::number::pkey_mprotect == SYS_pkey_mprotect);
#	endif
#	if defined(SYS_pkey_alloc)
static_assert(ca::syscall::number::pkey_alloc == SYS_pkey_alloc);
#	endif
#	if defined(SYS_pkey_free)
static_assert(ca::syscall::number::pkey_free == SYS_pkey_free);
#	endif
#	if defined(SYS_statx)
static_assert(ca::syscall::number::statx == SYS_statx);
#	endif
#	if defined(SYS_io_pgetevents)
static_assert(ca::syscall::number::io_pgetevents == SYS_io_pgetevents);
#	endif
#	if defined(SYS_rseq)
static_assert(ca::syscall::number::rseq == SYS_rseq);
#	endif
#	if defined(SYS_uretprobe)
static_assert(ca::syscall::number::uretprobe == SYS_uretprobe);
#	endif
#	if defined(SYS_uprobe)
static_assert(ca::syscall::number::uprobe == SYS_uprobe);
#	endif
#	if defined(SYS_pidfd_send_signal)
static_assert(ca::syscall::number::pidfd_send_signal == SYS_pidfd_send_signal);
#	endif
#	if defined(SYS_io_uring_setup)
static_assert(ca::syscall::number::io_uring_setup == SYS_io_uring_setup);
#	endif
#	if defined(SYS_io_uring_enter)
static_assert(ca::syscall::number::io_uring_enter == SYS_io_uring_enter);
#	endif
#	if defined(SYS_io_uring_register)
static_assert(ca::syscall::number::io_uring_register == SYS_io_uring_register);
#	endif
#	if defined(SYS_open_tree)
static_assert(ca::syscall::number::open_tree == SYS_open_tree);
#	endif
#	if defined(SYS_move_mount)
static_assert(ca::syscall::number::move_mount == SYS_move_mount);
#	endif
#	if defined(SYS_fsopen)
static_assert(ca::syscall::number::fsopen == SYS_fsopen);
#	endif
#	if defined(SYS_fsconfig)
static_assert(ca::syscall::number::fsconfig == SYS_fsconfig);
#	endif
#	if defined(SYS_fsmount)
static_assert(ca::syscall::number::fsmount == SYS_fsmount);
#	endif
#	if defined(SYS_fspick)
static_assert(ca::syscall::number::fspick == SYS_fspick);
#	endif
#	if defined(SYS_pidfd_open)
static_assert(ca::syscall::number::pidfd_open == SYS_pidfd_open);
#	endif
#	if defined(SYS_clone3)
static_assert(ca::syscall::number::clone3 == SYS_clone3);
#	endif
#	if defined(SYS_close_range)
static_assert(ca::syscall::number::close_range == SYS_close_range);
#	endif
#	if defined(SYS_openat2)
static_assert(ca::syscall::number::openat2 == SYS_openat2);
#	endif
#	if defined(SYS_pidfd_getfd)
static_assert(ca::syscall::number::pidfd_getfd == SYS_pidfd_getfd);
#	endif
#	if defined(SYS_faccessat2)
static_assert(ca::syscall::number::faccessat2 == SYS_faccessat2);
#	endif
#	if defined(SYS_process_madvise)
static_assert(ca::syscall::number::process_madvise == SYS_process_madvise);
#	endif
#	if defined(SYS_epoll_pwait2)
static_assert(ca::syscall::number::epoll_pwait2 == SYS_epoll_pwait2);
#	endif
#	if defined(SYS_mount_setattr)
static_assert(ca::syscall::number::mount_setattr == SYS_mount_setattr);
#	endif
#	if defined(SYS_quotactl_fd)
static_assert(ca::syscall::number::quotactl_fd == SYS_quotactl_fd);
#	endif
#	if defined(SYS_landlock_create_ruleset)
static_assert(ca::syscall::number::landlock_create_ruleset == SYS_landlock_create_ruleset);
#	endif
#	if defined(SYS_landlock_add_rule)
static_assert(ca::syscall::number::landlock_add_rule == SYS_landlock_add_rule);
#	endif
#	if defined(SYS_landlock_restrict_self)
static_assert(ca::syscall::number::landlock_restrict_self == SYS_landlock_restrict_self);
#	endif
#	if defined(SYS_memfd_secret)
static_assert(ca::syscall::number::memfd_secret == SYS_memfd_secret);
#	endif
#	if defined(SYS_process_mrelease)
static_assert(ca::syscall::number::process_mrelease == SYS_process_mrelease);
#	endif
#	if defined(SYS_futex_waitv)
static_assert(ca::syscall::number::futex_waitv == SYS_futex_waitv);
#	endif
#	if defined(SYS_set_mempolicy_home_node)
static_assert(ca::syscall::number::set_mempolicy_home_node == SYS_set_mempolicy_home_node);
#	endif
#	if defined(SYS_cachestat)
static_assert(ca::syscall::number::cachestat == SYS_cachestat);
#	endif
#	if defined(SYS_fchmodat2)
static_assert(ca::syscall::number::fchmodat2 == SYS_fchmodat2);
#	endif
#	if defined(SYS_map_shadow_stack)
static_assert(ca::syscall::number::map_shadow_stack == SYS_map_shadow_stack);
#	endif
#	if defined(SYS_futex_wake)
static_assert(ca::syscall::number::futex_wake == SYS_futex_wake);
#	endif
#	if defined(SYS_futex_wait)
static_assert(ca::syscall::number::futex_wait == SYS_futex_wait);
#	endif
#	if defined(SYS_futex_requeue)
static_assert(ca::syscall::number::futex_requeue == SYS_futex_requeue);
#	endif
#	if defined(SYS_statmount)
static_assert(ca::syscall::number::statmount == SYS_statmount);
#	endif
#	if defined(SYS_listmount)
static_assert(ca::syscall::number::listmount == SYS_listmount);
#	endif
#	if defined(SYS_lsm_get_self_attr)
static_assert(ca::syscall::number::lsm_get_self_attr == SYS_lsm_get_self_attr);
#	endif
#	if defined(SYS_lsm_set_self_attr)
static_assert(ca::syscall::number::lsm_set_self_attr == SYS_lsm_set_self_attr);
#	endif
#	if defined(SYS_lsm_list_modules)
static_assert(ca::syscall::number::lsm_list_modules == SYS_lsm_list_modules);
#	endif
#	if defined(SYS_mseal)
static_assert(ca::syscall::number::mseal == SYS_mseal);
#	endif
#	if defined(SYS_setxattrat)
static_assert(ca::syscall::number::setxattrat == SYS_setxattrat);
#	endif
#	if defined(SYS_getxattrat)
static_assert(ca::syscall::number::getxattrat == SYS_getxattrat);
#	endif
#	if defined(SYS_listxattrat)
static_assert(ca::syscall::number::listxattrat == SYS_listxattrat);
#	endif
#	if defined(SYS_removexattrat)
static_assert(ca::syscall::number::removexattrat == SYS_removexattrat);
#	endif
#	if defined(SYS_open_tree_attr)
static_assert(ca::syscall::number::open_tree_attr == SYS_open_tree_attr);
#	endif
#	if defined(SYS_file_getattr)
static_assert(ca::syscall::number::file_getattr == SYS_file_getattr);
#	endif
#	if defined(SYS_file_setattr)
static_assert(ca::syscall::number::file_setattr == SYS_file_setattr);
#	endif
#	if defined(SYS_listns)
static_assert(ca::syscall::number::listns == SYS_listns);
#	endif

#endif
