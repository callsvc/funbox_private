#include <time.h>
#include <unistd.h>
#include <types.h>

procinfo_t * procinfo_create() {
    procinfo_t *procinfo = fb_malloc(sizeof(procinfo_t));
    getcwd(procinfo->current_dir, sizeof(procinfo->current_dir));

    procinfo->start = time(nullptr);
    return procinfo;
}

void procinfo_destroy(procinfo_t *procinfo) {
    fb_free(procinfo);
}