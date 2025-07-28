#include <dospc.h>

int main() {
    dospc_t *oldpc = dospc_create();
    dospc_reset(oldpc);

    dospc_continue(oldpc);

    dospc_destroy(oldpc);
}
