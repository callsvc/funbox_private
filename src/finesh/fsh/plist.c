#include <string.h>
#include <libxml/xmlreader.h>
#include <fsh/plist.h>
#include <types.h>


int32_t cmpstrs(const char *str, const char **list) {
    int32_t count = 0;
    for (const char **sp = list; *sp; ++sp) {
        if (!strcmp(str, *sp))
            count++;
    }
    return count;
}

void plist_getval(plist_value_t *value, const xmlNodePtr node_ptr) {
    const char * valuestr = (char*)xmlNodeGetContent(node_ptr);
    if (strlen(valuestr))
        strcpy(value->string, valuestr);
    else
        strcpy(value->string, (const char*)node_ptr->name);
    xmlFree((char*)valuestr);
}

plist_value_t* plist_pvalue(plist_value_t *value) {
    if (!value->list) {
        value->list = list_create(sizeof(plist_value_t));
    }
    plist_value_t * val = list_emplace(value->list);
    val->parent = value;
    return val;
}

void plist_parser(plist_t *plist, xmlNodePtr cur_node, plist_value_t *value) {

    for (; cur_node; cur_node = cur_node->next) {
        const char *nodename = (char*)cur_node->name;
        const char *properties[] = {"plist", "dict", "array", nullptr};
        if (cmpstrs(nodename, properties)) {
            value = plist_pvalue(value);
        }
        if (cur_node->type == XML_ELEMENT_NODE)
            plist_parser(plist, cur_node->children, value->list ? plist_pvalue(value) : value);

        const char * values[] = {"integer", "true", "false", "key", "string", nullptr};
        if (!cmpstrs(nodename, values))
            continue;
        plist_getval(value, cur_node);
        value = plist_pvalue(value->parent);
    }
}

plist_t * plist_create(fsfile_t *file) {
    plist_t * plist = fb_malloc(sizeof(plist_t));

    vector_t *content = fs_getbytes(file, fs_getsize(file), 0);
    const xmlDocPtr rootxml = xmlReadMemory(vector_begin(content), vector_size(content), nullptr, nullptr, 0);

    const xmlNodePtr root = xmlDocGetRootElement(rootxml);
    if (strcmp((const char*)root->name, "plist"))
        return nullptr;

    plist->root = fb_malloc(sizeof(plist_value_t));

    plist_parser(plist, root, plist->root);
    vector_destroy(content);
    xmlFreeDoc(rootxml);

    return plist;
}

plist_value_t *plist_search(const plist_value_t * val, const char * key) {
    for (size_t i = 0; i < list_size(val->list); i++) {
        const plist_value_t * eval = list_get(val->list, i), * result;
        if (!eval->list && !strcmp(eval->string, key))
            return list_get(val->list, i + 1);
        if ((result = plist_search(eval, key)))
            return (plist_value_t*)result;
    }
    return nullptr;
}

const char * plist_getstr(const plist_t *plist, const char *key) {
    return plist_search(plist->root, key)->string;
}

void plist_print_depth(const int32_t depth, const plist_value_t * value) {
    if (!value->list)
        printf("%*s%s\n", depth * 4, "", value->string);
    for (size_t i = 0; value->list && i < list_size(value->list); i++)
        plist_print_depth(depth + 1, list_get(value->list, i));

}

void plist_value_cleanup(const plist_value_t *value) {
    for (size_t i = 0; value->list && i < list_size(value->list); i++) {
        plist_value_cleanup(list_get(value->list, i));
    }
    if (value->list)
        list_destroy(value->list);
}

void plist_destroy(plist_t *plist) {
    if (plist->root) {
        plist_print_depth(0, plist->root);
    }
    plist_value_cleanup(plist->root);
    fb_free(plist->root);

    fb_free(plist);
}