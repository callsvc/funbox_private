#include <stdlib.h>
#include <string.h>

#include <cycles.h>
cycles_t * cycles_create(sony_t * sony) {
    cycles_t * cycles = funbox_malloc(sizeof(cycles_t));
    cycles->sony = sony;
    cycles->affinity_isrng = true;

    const device_step_t devices_list[] = {
        [device_type_ee] = {.target_dev = sony->mips, .stepdev = (device_step_func_t)ee_run, .ticks = &cycles->ee_cycles},
        [device_type_iop] = {.target_dev = sony->iop, .stepdev = (device_step_func_t)mips_run, .ticks = &cycles->iop_cycles},
        [device_type_dmac] ={.target_dev = sony->dmac, .stepdev = (device_step_func_t)dmac_run, &cycles->dmac_cycles}
    };

    const size_t devs_count = count_of(devices_list);
    cycles->devices = funbox_malloc(sizeof(void *) * devs_count);
    cycles->affinity = funbox_malloc(sizeof(device_type_e) * devs_count);

    for (size_t i = 0; i < devs_count; i++) {
        cycles->devices[i] = funbox_malloc(sizeof(device_step_t));
        memcpy(cycles->devices[i], &devices_list[i], sizeof(devices_list[i]));

        cycles->devices_count++;
    }
    return cycles;
}

static const char * affinity_default_strs[] = {
    [affinity_default_mips_first] = "ee,iop,dmac"
};

void cycles_set_affinity_default(cycles_t *cycles, const affinity_default_type_e type) {
    cycles_set_affinity(cycles, affinity_default_strs[type]);
}

void cycles_set_affinity(cycles_t *cycles, const char * str) {
    device_type_e *devs = cycles->affinity;
    cycles->affinity_isrng = false;

    char *strdevs = strdup(str);
    char *tok;
    for (const char * dev = strtok_r(strdevs, ",", &tok); dev; dev = strtok_r(nullptr, ",", &tok))
        if (!strcmp(dev, "ee"))
            *devs++ = device_type_ee;
        else if (!strcmp(dev, "iop"))
            *devs++ = device_type_iop;
        else if (!strcmp(dev, "dmac"))
            *devs++ = device_type_dmac;
    funbox_free(strdevs);
}

const char * cycles_get_affinity_desc(const affinity_default_type_e type) {
    if (type == affinity_default_mips_first)
        return "mips first, iop 1/4 of ee";
    return nullptr;
}

const char * cycles_get_affinity_str(const cycles_t *cycles) {
    static _Thread_local char buffer[0x45];
    if (cycles->affinity_isrng)
        return "is rng";
    *buffer = '?';

    for (size_t i = 0; i < cycles->devices_count; i++)
        switch (cycles->affinity[i]) {
            case device_type_ee:
                sprintf(strchr(buffer, '?'), "ee,?"); break;
            case device_type_iop:
                sprintf(strchr(buffer, '?'), "iop,?"); break;
            case device_type_dmac:
                sprintf(strchr(buffer, '?'), "dmac,?"); break;
        }
    *(strchr(buffer, '?') - 1) = '\0';

    for (size_t i = 0; i < count_of(affinity_default_strs); i++)
        if (!strcmp(buffer, affinity_default_strs[i]))
            if (sprintf(buffer + strlen(buffer) - 1, " $(%s)", cycles_get_affinity_desc(i)))
                break;

    return buffer;
}

void cycles_destroy(cycles_t * cycles) {
    for (size_t i = 0; i < cycles->devices_count; i++)
        funbox_free(cycles->devices[i]);

    funbox_free(cycles->devices);
    funbox_free(cycles->affinity);

    funbox_free(cycles);
}

void cycles_step_device(cycles_t * cycles, const device_type_e device) {
    const device_step_t * device_run = cycles->devices[device];
    if (device_run->target_dev)
        device_run->stepdev(device_run->target_dev, device_run->ticks);

    cycles->last_executed = device;
}

void cycles_step_devs(cycles_t * cycles) {
    cycles->ee_cycles += 16;
    cycles->iop_cycles = cycles->ee_cycles / 4;
    cycles->dmac_cycles = cycles->ee_cycles;

    cycles->count += cycles->ee_cycles;

    if (!cycles->affinity_isrng) {
        for (size_t i = 0; i < cycles->devices_count; i++)
            cycles_step_device(cycles, cycles->affinity[i]);
    }
    const bool ee_can_execute = cycles->last_executed != device_type_dmac;

    if (funbox_rand() % (cycles->count / 16) && ee_can_execute)
        cycles_step_device(cycles, device_type_ee);
    if (funbox_rand() % (cycles->count / 4))
        cycles_step_device(cycles, device_type_iop);
    if (funbox_rand() % (cycles->count / 16))
        cycles_step_device(cycles, device_type_dmac);
}