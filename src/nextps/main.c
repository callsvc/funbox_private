#include <ps.h>

cpu_t *cpu;
bus_t *bus;

void reset() {
    cpu_reset(cpu);
}

int main() {
    bus = bus_create();
    cpu = cpu_create();
    reset();
    cpu_destroy(cpu);
    bus_destroy(bus);
}
