#ifndef __TELEIOS_CONTAINER__
#define __TELEIOS_CONTAINER__

#include "teleios/defines.h"

typedef struct TLNode {
    void *payload;
    struct TLNode* next;
    struct TLNode* previous;
} TLNode;

typedef struct {
    u64 length;
    TLNode* payload;
} TLList;

TLList* tl_list_create(void);
void tl_list_destroy(TLList* list);
void tl_list_clear(TLList* list, void (*PFN_dealocator)(void *pointer));
void *tl_list_search(TLList* list, b8 (PFN_filter)(void *pointer));
void tl_list_remove(TLList* list, void *pointer);
void tl_list_insert_add(TLList* list, void *pointer);
void tl_list_insert_insert(TLList* list, void *item, void *pointer);

#endif // __TELEIOS_CONTAINER__