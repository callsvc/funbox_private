#include <core/types.h>
#include <sony.h>
#include <fs/types.h>

sony_t * sony_create() {
    sony_t *sony = funbox_malloc(sizeof(sony_t));
    sony->procinfo = procinfo_create();

    sony->unkpaddr = vector_create(0, sizeof(uint32_t));
    sony->firmpath = fs_build_path(2, sony->procinfo->proc_cwd, "scph10000.bin");

    sony->bridge = bridge_create(sony);
    sony->mips = ee_create(sony->bridge);
    sony->iop = mips_create(sony->bridge);
    sony->dmac = dmac_create(sony->mips);

    return sony;
}

void sony_reset_cpus(const sony_t *sony) {
    ee_reset(sony->mips);
    mips_reset(sony->iop);
}

void sony_destroy(sony_t *sony) {
    dmac_destroy(sony->dmac);
    ee_destroy(sony->mips);
    mips_destroy(sony->iop);
    if (sony->firmpath)
        funbox_free((char*)sony->firmpath);

    procinfo_destroy(sony->procinfo);
    vector_destroy(sony->unkpaddr);
    bridge_destroy(sony->bridge);
    funbox_free(sony);

}
