#pragma once

#include <libxml/xmlreader.h>
#include <fs/types.h>
typedef struct plist {
    xmlDocPtr pxml;
} plist_t;

plist_t * plist_create(fsfile_t *);
void plist_destroy(plist_t *);