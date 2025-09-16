#include <types.h>
#include <algo/list.h>

list_t *list_create(const size_t typesize) {
    list_t * list = fb_malloc(sizeof(list_t));
    list->size = typesize;
    list->heap = mi_heap_new();
    return list;
}

static void insert(list_t *list, list_node_t *listnode) {
#if 0
    for (list_t *node = begin; node; node = node->next)
        if (node->next == nullptr)
            if ((node->next = listnode))
                break;
#else
    if (!list->tail)
        list->tail = listnode;
    if (list->head)
        list->head->next = listnode;
    list->head = listnode;
#endif
}

void * list_emplace(list_t *list) {
    if (!list->size)
        return nullptr;
    list_node_t *listnode = mi_heap_zalloc(list->heap, sizeof(list_node_t) + list->size);
    listnode->data = (uint8_t*)listnode + sizeof(list_node_t);

    insert(list, listnode);
    return listnode->data;
}
void list_push(list_t *list, void *data) {
    list_node_t *listnode = mi_heap_zalloc(list->heap, sizeof(list_node_t));
    listnode->data = data;
    insert(list, listnode);
}
size_t list_size(const list_t * list) {
    size_t size = 0;
    for (const list_node_t *ln = list->tail; ln; ln = ln->next)
        if (ln->data)
            size++;
    return size;
}

size_t list_locate(const list_t *list, const void *data) {
    const list_node_t *node = list->tail;
    for (size_t index = 0; node; index++)
        if (node->data != data)
            node = node->next;
        else return index;
    return 0;
}

void list_drop(list_t *list, const size_t index) {
    list_node_t *node = list->tail;
    list_node_t *prev = nullptr;

    for (size_t i = 0; i < index && node; i++) {
        prev = node;
        node = node->next;
    }
    if (node && node->next)
        prev->next = node->next;
    else if (prev) prev->next = nullptr;

    if (list->head == node)
        list->head = prev;
    if (list->tail == node)
        list->tail = list->tail->next ? list->tail->next : nullptr;
}

void * list_get(const list_t *list, size_t index) {
    const list_node_t *node = list->tail;
    for (; node && index; index--)
        node = node->next;
    return node ? node->data : nullptr;
}
void list_destroy(list_t *list) {
    mi_heap_destroy(list->heap);
    fb_free(list);
}