#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <types.h>

char username[30] = {};
void envos_issafe() {
    const uid_t euid = geteuid();
    if (euid == 0)
        oskill("You can't run as a root user!");
}
void funbox_create() {
    strcpy(username, getenv("HOME"));
}

void funbox_destroy() {
}
