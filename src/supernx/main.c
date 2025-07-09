

#include <nx_sys.h>
int main() {
    nx_sys_t *nx_sys = nx_sys_create();
    nx_get_all_loaders(nx_sys);

    if (nx_get_games_count(nx_sys))
        nx_load_first_one(nx_sys);

    nx_sys_destroy(nx_sys);
}
