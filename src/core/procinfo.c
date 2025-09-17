#include <time.h>
#include <unistd.h>
#include <types.h>

procinfo_t * procinfo_create() {
    procinfo_t *procinfo = fb_malloc(sizeof(procinfo_t));
    if (getcwd(procinfo->current_dir, sizeof(procinfo->current_dir)) == nullptr)
        quit("size of destination buffer");

    procinfo->start = time(nullptr);
    return procinfo;
}

void procinfo_destroy(procinfo_t *procinfo) {
    fb_free(procinfo);
}