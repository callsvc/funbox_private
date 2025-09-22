#include <stdio.h>
#include <algo/robin.h>

int main() {
    uint8_t types[] = {0, 0};
    robin_map_t * robin_map = robin_map_create(types);
    robin_map_emplace(robin_map, "meu nome", "Gabriel Correia");
    robin_map_emplace(robin_map, "comida favorita", "lasanha");
    robin_map_emplace(robin_map, "cor dahora", "azul");
    robin_map_print(robin_map);
    robin_map_emplace(robin_map, "expedição", "33");

    robin_map_print(robin_map);
    fprintf(stderr, "razão universal: %s\n", (const char*)robin_map_get(robin_map, "expedição"));

    robin_map_destroy(robin_map);
}