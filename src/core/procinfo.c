#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <types.h>

procinfo_t * procinfo_create() {
    procinfo_t *procinfo = funbox_malloc(sizeof(procinfo_t));
    getcwd(procinfo->proc_cwd, sizeof(procinfo->proc_cwd));

    procinfo->start = time(NULL);
    return procinfo;
}

void procinfo_destroy(procinfo_t *procinfo) {
    funbox_free(procinfo);
}