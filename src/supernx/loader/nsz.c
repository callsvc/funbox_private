#include <assert.h>
#include <string.h>

#include <zstd.h>
#include <mbedtls/sha256.h>

#include <types.h>
#include <fs_fmt/offset_file.h>
#include <fs_fmt/content_archive.h>
#include <fs_fmt/aes_file.h>
#include <fs_fmt/pfs.h>

#include <fs/mapfile.h>
#include <horizon/tik.h>
#include <loader/nsz.h>



bool is_ncz(fsfile_t *file) {
    uint8_t magic[8];
    fs_read(file, magic, 8, 0x4000);
    return memcmp(magic, "NCZSECTN", 8) == 0;
}

ncz_t * ncz_create(fsfile_t *file, fsfile_t **ncafile, const bool verify) {
    if (!is_ncz(file))
        return nullptr;

    ncz_t * ncz = fb_malloc(sizeof(ncz_t));
    fs_read(file, &ncz->sections_size, 8, 0x4008);

    ncz->sections = fb_malloc(sizeof(ncz_section_t) * ncz->sections_size);
    for (size_t i = 0; i < ncz->sections_size; i++) {
        fs_read(file, ncz->sections + i, sizeof(*ncz->sections), 0x4008 + 0x8 + sizeof(*ncz->sections) * i);
        ncz->nca_size += ncz->sections[i].size;
    }

    uint8_t * file_content = fb_malloc(ncz->nca_size + 0x4000);
    ZSTD_DCtx* dctx = ZSTD_createDCtx();

    fs_read(file, file_content, 0x4000, 0);

    ZSTD_outBuffer output = { file_content, ncz->nca_size + 0x4000, 0x4000};

    size_t offset = 0x4008 + 0x8 + ncz->sections_size * sizeof(ncz_section_t);

    uint8_t *zcnt = fb_malloc(64 * 1024);
    size_t section_offset = ncz->sections->offset;

    const fsfile_t *backing = (fsfile_t*)mapfile_open_2(fs_getpath(file), file_content, ncz->nca_size + 0x4000);
    size_t encrypted = 0x4000;

    *ncafile = (fsfile_t*)aes_file_open((fsfile_t*)backing, aes_type_ctr128, "w");
    for (size_t i = 0; i < ncz->sections_size && !ncz->blocks; i++) {

        aes_file_setkey((aes_file_t*)*ncafile, ncz->sections[i].crypto_key, 16);
        aes_file_setiv((aes_file_t*)*ncafile, ncz->sections[i].crypto_iv);

        while (section_offset < ncz->sections[i].offset + ncz->sections[i].size) {
            size_t readsize = MIN((ncz->sections[i].offset + ncz->sections[i].size) - section_offset, 64 * 1024);
            if (MIN(readsize, fs_getsize(file) - offset) != readsize)
                readsize = MIN(readsize, fs_getsize(file) - offset);

            fs_read(file, zcnt, readsize, offset);
            ZSTD_inBuffer input = {zcnt, readsize};
            offset += readsize;

            const size_t result = ZSTD_decompressStream(dctx, &output, &input);
            if (ZSTD_isError(result))
                quit("Zstandard errorstr: %s", ZSTD_getErrorName(result));
            section_offset = output.pos;
        }
        // err = 0, none = 1, xts = 2, ctr = 3, bktr = 4
        encrypted += ncz->sections[i].size;
        if (ncz->sections[i].crypto_type < 3)
            continue;
        fs_write(*ncafile, nullptr, ncz->sections[i].size, ncz->sections[i].offset);
    }
    aes_file_close((aes_file_t*)*ncafile);
    fb_free(zcnt);

    *ncafile = (fsfile_t*)backing;
    if (verify) {
        uint8_t result[32];
        char str[64];

        fs_sha256(*ncafile, result);
        to_str(str, result, sizeof(result));
        strcpy(str + 32, ".ncz");
        if (encrypted != ncz->nca_size + 0x4000 || strcmp(fs_getpath(file), str) != 0)
            quit("data corruption detected");
    }

    ZSTD_freeDCtx(dctx);
    return ncz;
}

vector_t * loader_nsz_get_logo(loader_base_t *base) {
    const auto nsz = (nsz_t*)base;
    for (size_t i = 0; i < list_size(nsz->nca_list); i++) {
        for (size_t j = 0; content_archive_get_fs(list_get(nsz->nca_list, i), j, false); j++) {
            romfs_t * ctrlfs = content_archive_get_fs(list_get(nsz->nca_list, i), j, false);
            fsfile_t *title_icon = fs_open_file((fsdir_t*)ctrlfs, "icon_AmericanEnglish.dat", "r");
            if (!title_icon)
                continue;
            vector_t * logo = fs_getfile(title_icon);
            return logo;
        }
    }
    return nullptr;
}
uint64_t loader_nsz_get_program_id(loader_base_t *base) {
    const content_archive_t *nca = list_get(((nsz_t*)base)->nca_list, 0);
    if (nca && nca->program_id > 0)
        return nca->program_id;
    return 0;
}

void ncz_destroy(ncz_t *ncz) {
    fb_free(ncz->sections);
    fb_free(ncz);
}

void nsz_addfile(const nsz_t *nsz, keys_db_t *keys, fsfile_t *file) {
    const char *ext = fs_getpath(file) + strlen(fs_getpath(file)) - 4;
    if (strcmp(ext, ".tik") == 0) {
        const tik_t *tik = tik_create(file);
        keys_db_add_ticket(keys, tik);
        return;
    }
    if (strcmp(ext, ".ncz") == 0) {
        fsfile_t * parent = nullptr;
        ncz_t * this_ncz = ncz_create(file, &parent, false);

        romfs_addfile(nsz->nca_files, parent);
        ncz_destroy(this_ncz);

    } else if (strcmp(ext, ".nca") == 0 && memcmp(ext - 5, ".cnmt", 5) != 0) {
        romfs_addfile(nsz->nca_files, file);
    } else {
        return;
    }

    list_push(nsz->nca_list, content_archive_create(keys, (fsdir_t*)nsz->nca_files, fs_getpath(file)));
}

nsz_t * nsz_create(fsfile_t *file, keys_db_t *keys) {

    nsz_t * nsz = fb_malloc(sizeof(nsz_t));
    nsz->main_pfs = pfs_create(file);
    nsz->nca_list = list_create(0);
    nsz->nca_files = romfs_create_2();

    vector_t *files = fs_list_all_files((fsdir_t*)nsz->main_pfs);
    for (size_t i = 0; i < vector_size(files); i++)
        nsz_addfile(nsz, keys, fs_open_file((fsdir_t*)nsz->main_pfs, vector_get(files, i), "r"));

    vector_destroy(files);

    nsz->vloader.loader_get_program_id = loader_nsz_get_program_id;
    nsz->vloader.loader_get_logo = loader_nsz_get_logo;

    return nsz;
}

void nsz_destroy(nsz_t *nsz) {
    for (size_t i = 0; i < list_size(nsz->nca_list); i++) {
        content_archive_destroy(list_get(nsz->nca_list, i));
    }
    list_destroy(nsz->nca_list);
    romfs_destroy(nsz->nca_files);
    pfs_destroy(nsz->main_pfs);
    fb_free(nsz);
}

bool nsz_is_nsz(fsfile_t *file) {
    return pfs_is_pfs(file);
}

