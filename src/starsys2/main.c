#include <sony.h>
#include <cycles.h>
int main() {
    sony_t *sony = sony_create();
    cycles_t * ticks_gen = cycles_create(sony);

    sony_reset_cpus(sony);
    cycles_set_affinity_default(ticks_gen, affinity_default_mips_first);

    const char * affinity_str = cycles_get_affinity_str(ticks_gen);
    printf("cpu affinity: (%s)\n", affinity_str);

    int64_t count = 10;
    for (;;) {
        cycles_step_devs(ticks_gen);
        sleep_for(ms(50));
        if (!count--)
            break;
    }

    cycles_destroy(ticks_gen);
    sony_destroy(sony);
}
