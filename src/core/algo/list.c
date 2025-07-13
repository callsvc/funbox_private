
#include <strings.h>
#include <types.h>
#include <algo/list.h>

list_t *list_create(const size_t typesize) {
    list_t * list = funbox_malloc(sizeof(list_t));
    list->size = typesize;
    return list;
}

static void insert(list_t *begin, list_t *listnode) {
    for (list_t *node = begin; node; node = node->next)
        if (node->next == nullptr)
            if ((node->next = listnode))
                break;
}

void * list_emplace(list_t *list) {
    if (!list->size)
        return nullptr;
    list_t *listnode = funbox_malloc(sizeof(list_t));
    listnode->data = funbox_malloc(list->size);
    insert(list, listnode);
    return listnode->data;
}
void list_push(list_t *list, void *data) {
    list_t *listnode = funbox_malloc(sizeof(list_t));
    listnode->data = data;
    insert(list, listnode);
}
size_t list_size(const list_t * list) {
    size_t size = 0;
    for (; list; list = list->next)
        if (list->data)
            size++;
    return size;
}

size_t list_locate(const list_t *list, const void *data) {
    const list_t *node = list->next;
    for (size_t index = 0; node; index++)
        if (node->data != data)
            node = node->next;
        else return index;
    return 0;
}

void list_drop(list_t *list, const size_t index) {
    list_t *node = list->next;
    list_t *prev = list;

    const bool allocated = list->size;
    for (size_t i = 0; i < index; i++) {
        prev = node;
        node = node->next;
    }
    if (node->next)
        prev->next = node->next;
    else prev->next = nullptr;

    if (allocated && node->data)
        funbox_free(node->data);
    funbox_free(node);
}

void * list_get(const list_t *list, size_t index) {
    const list_t *node = list->next;
    for (; node && index; index--)
        node = node->next;
    return node ? node->data : nullptr;
}
void list_destroy(list_t *list) {
    list_t *next = list;
    const bool isfreed = next->size == 0;
    do {
        list_t *save = next->next;
        if (!isfreed)
            funbox_free(next->data);
        funbox_free(next);
        next = save;
    } while (next);
}