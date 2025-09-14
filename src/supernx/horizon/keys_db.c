#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <types.h>
#include <horizon/tik.h>
#include <horizon/keys_db.h>

void keys_compile_regex(regex_t *regex, const char *exp) {
    if (regcomp(regex, exp, REG_EXTENDED) != 0)
        quit("can't compile regex");
}


const char * named_keys[] = {"header_key"};

keys_db_t *keys_db_create() {
    keys_db_t *kdb = fb_malloc(sizeof(keys_db_t));
    kdb->keys_path = list_create(0);
    kdb->titles = set_create();
    kdb->tickets = list_create(0);

    kdb->named_keys = ht_create(count_of(named_keys), sizeof(key256_t), named_keys);

    kdb->tag_keysmap = ht_create(0, sizeof(tagged_key_t), nullptr);

    keys_compile_regex(&kdb->prod_regex, "^\\w+\\s*=\\s*[a-fA-F0-9]+$");
    keys_compile_regex(&kdb->title_regex, "^[a-fA-F0-9]{32}\\s*=\\s*[a-fA-F0-9]{32}$");
    return kdb;
}

static char* get_keyline(char *output, const char * line) {
    char *key_val = strchr(line, '=') + 2;
    fb_strcopy(output, line, strchr(line, '=') - line - 1);
    return key_val;
}

void keys_add_title(keys_db_t *kdb, const char *line) {
    kdb->count++;

    char keyname[100] = {};
    const char * key_val = get_keyline(keyname, line);
    // logger_info("new key %s = %s", keyname, key_val);
    set_set(kdb->titles, setval_string, setval_string, keyname, key_val);
}

static void insert_prod256(keys_db_t *kdb, const char * key, const char * value) {
    tagged_key_t keyval = {};

    if (ht_contains(kdb->named_keys, key)) {
        strtobytes(value, &keyval.value, sizeof(keyval.value));
        ht_insert(kdb->named_keys, key, &keyval.value);

        if (strcmp(key, "header_key") == 0)
            kdb->header_key = ht_get(kdb->named_keys, "header_key");
        return;
    }

    static const char *keys_area[] = {"none key wtf?!!", "titlekek_", "key_area_key_application_", "key_area_key_ocean_", "key_area_key_system_"};

    key_type_e type = key_none;
    for (size_t i = 0; i < count_of(keys_area) && keyval.type == key_none; i++) {
        if (strncmp(key, keys_area[i], strlen(keys_area[i])) == 0)
            keyval.type = type;
        else type++;
    }
    if (keyval.type == key_none)
        return;

    strtobytes(value, &keyval.indexed, sizeof(keyval.indexed));
    const char * index = strrchr(key, '_') + 1;
    if (!index)
        quit("index not found for indexable key %s", key);
    keyval.index = strtoul(index, nullptr, 16);
    if (!ht_contains(kdb->tag_keysmap, key))
        ht_insert(kdb->tag_keysmap, key, &keyval);
}

void keys_add_prod(keys_db_t *kdb, const char *line) {
    kdb->count++;
    char keyname[100] = {};
    const char * key_val = get_keyline(keyname, line);

    bool inserted = false;
    for (size_t i = 0; i < count_of(named_keys) && !inserted; i++) {
        if (strcmp(keyname, named_keys[i]) != 0)
            continue;
        insert_prod256(kdb, keyname, key_val);
        inserted = true;
    }

    if (!inserted && strstr(keyname, "_") && isdigit(*(keyname + strlen(keyname) - 2)))
        insert_prod256(kdb, keyname, key_val);
}

void keys_db_load(keys_db_t *kdb, fsfile_t *file) {
    list_push(kdb->keys_path, (void*)fs_getpath(file));
    vector_t *content = fs_filebytes(file);

    const char *line = vector_begin(content);
    const char *end = line + vector_size(content);
    do {
        static char keypair[1 << 11] = {};
        const char *last = strchr(line, '\n');
        fb_strcopy(keypair, line, last ? last - line : strlen(line));

        if (regexec(&kdb->title_regex, keypair, 0, nullptr, 0) == 0)
            keys_add_title(kdb, keypair);
        else if (regexec(&kdb->prod_regex, keypair, 0, nullptr, 0) == 0)
            keys_add_prod(kdb, keypair);
        line = strchr(line, '\n');
        if (!line)
            break;
        line++;
    } while (line != end);
    vector_destroy(content);
}

void keys_db_add_ticket(const keys_db_t *kdb, const tik_t *tik) {
    for (size_t i = 0; i < list_size(kdb->tickets); i++)
        if (tik_isequal(list_get(kdb->tickets, i), tik))
            return;

    list_push(kdb->tickets, (void*)tik);
}
void keys_db_get_titlekey(const keys_db_t *kdb, key128_t *key_dest, const key128_t *rights_id) {
    memset(key_dest, 0, sizeof(*key_dest));
    for (size_t i = 0; i < list_size(kdb->tickets); i++) {
        const tik_t *ticket = list_get(kdb->tickets, i);
        if (tik_gettitle(ticket, (uint8_t*)key_dest, (const uint8_t*)rights_id))
            return;
    }
}

void keys_db_destroy(keys_db_t *kdb) {
    regfree(&kdb->title_regex);
    regfree(&kdb->prod_regex);

    for (size_t i = 0; i < list_size(kdb->tickets); i++) {
        tik_destroy(list_get(kdb->tickets, i));
    }
    list_destroy(kdb->tickets);

    set_destroy(kdb->titles);
    list_destroy(kdb->keys_path);
    ht_destroy(kdb->named_keys);
    ht_destroy(kdb->tag_keysmap);
    fb_free(kdb);
}
