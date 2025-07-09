#include <sony.h>

int main() {
    sony_t *console = sony_create();

    sony_reset_cpus(console);

    int64_t count = 2;
    for (;;) {
        ee_run(console->mips);
        mips_run(console->iop);
        dmac_run(console->dmac);
        sleep_for(ms(500));
        if (!count--)
            break;
    }

    sony_destroy(console);
}
