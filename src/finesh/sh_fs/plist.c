#include <types.h>
#include <sh_fs/plist.h>

void plist_parser(plist_t *plist, const xmlNode *node) {
    for (const xmlNode * cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            printf("node type: element, name: %s\n", (const char*)cur_node->name);
            xmlChar *content = xmlNodeGetContent(cur_node);
            if (content)
                printf(" content: %s\n", (const char*)content);
            xmlFree(content);
        }
        plist_parser(plist, cur_node->children);
    }
}

plist_t * plist_create(fsfile_t *file) {
    plist_t * plist = fb_malloc(sizeof(plist_t));

    vector_t *content = fs_getbytes(file, fs_getsize(file), 0);
    plist->pxml = xmlReadMemory(vector_begin(content), vector_size(content), nullptr, nullptr, 0);

    const xmlNode *root = xmlDocGetRootElement(plist->pxml);
    plist_parser(plist, root);

    vector_destroy(content);

    return plist;
}
void plist_destroy(plist_t *plist) {
    if (plist->pxml)
        xmlFreeDoc(plist->pxml);
    fb_free(plist);
}