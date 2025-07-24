#include <ps.h>
#include <cpu.h>
void reset(cpu_t *cpu) {
    cpu_reset(cpu);
}

int main() {
    bus_t *bus = bus_create();
    cpu_t *cpu = cpu_create(bus);

    reset(cpu);
    while (!cpu->maskint) {
        cpu_run(cpu);
        if (cpu->pc == 0xBFC80000)
            break;
    }

    cpu_destroy(cpu);
    bus_destroy(bus);
}
