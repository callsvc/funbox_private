#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if NDEBUG
#include <seccomp.h>
#endif

#include <types.h>
#include <logger.h>
#include <storage.h>
char username[30] = {};

void fb_create() {
    const uid_t euid = geteuid();
    if (euid == 0)
        quit("You can't run as a root user!");
    strcpy(username, getenv("HOME"));
    logger_init();
    storage_init();

#if NDEBUG
    const scmp_filter_ctx filter = seccomp_init(SCMP_ACT_KILL);
    // read the kernel logs to enable upcoming syscalls
    constexpr uint32_t sys_allowed[] = {SCMP_SYS(read), SCMP_SYS(write), SCMP_SYS(open), SCMP_SYS(close), SCMP_SYS(fstat), SCMP_SYS(getcwd), SCMP_SYS(access), SCMP_SYS(openat), SCMP_SYS(lseek), SCMP_SYS(brk), SCMP_SYS(getdents64), SCMP_SYS(mmap), SCMP_SYS(munmap), SCMP_SYS(exit_group)};
    for (size_t i = 0; i < count_of(sys_allowed); i++) {
        seccomp_rule_add(filter, SCMP_ACT_ALLOW, sys_allowed[i], 0);
    }

    seccomp_load(filter);
    seccomp_release(filter);
#endif

}

void fb_destroy() {
    logger_destroy();
    storage_destroy();
}
