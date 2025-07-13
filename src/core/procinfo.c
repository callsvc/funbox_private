#include <time.h>
#include <unistd.h>
#include <types.h>

procinfo_t * procinfo_create() {
    procinfo_t *procinfo = fb_malloc(sizeof(procinfo_t));
    getcwd(procinfo->proc_cwd, sizeof(procinfo->proc_cwd));

    procinfo->start = time(nullptr);
    return procinfo;
}

void procinfo_destroy(procinfo_t *procinfo) {
    fb_free(procinfo);
}