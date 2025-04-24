// #include "libk/libk.h"
// // #####################################################################################################################
// //
// //                                                     LIST
// //
// // #####################################################################################################################
//
// struct TLNode {
//     void *payload;
//     struct TLNode* next;
//     struct TLNode* previous;
// };
//
// struct KList {
//     u64 length;
//     struct TLNode* head;
//     struct TLNode* tail;
//     KAllocator *allocator;
// } ;
//
// static struct TLNode* k_list_create_node(KAllocator *allocator, void *value) {
//     K_FRAME_PUSH_WITH("0x%p, 0X%p", allocator, value)
//
//     struct TLNode* created = k_memory_allocator_alloc(allocator, sizeof(struct TLNode), K_MEMORY_CONTAINER_NODE);
//     if (created == NULL) KFATAL("Failed to allocate struct TLNode")
//     created->payload = value;
//
//     K_FRAME_POP_WITH(created)
// }
//
// KList* k_list_create(KAllocator *allocator) {
//     K_FRAME_PUSH_WITH("0x%p", allocator)
//     KList* list = k_memory_allocator_alloc(allocator, sizeof(KList), K_MEMORY_CONTAINER_LIST);
//     if (list == NULL) {
//         KERROR("Failed to allocate KList")
//         K_FRAME_POP_WITH(NULL)
//     }
//
//     list->allocator = allocator;
//     list->length = 0;
//
//     K_FRAME_POP_WITH(list)
// }
//
// u64 k_list_length(KList* list) {
//     K_FRAME_PUSH_WITH("0x%p", list)
//     K_FRAME_POP_WITH(list->length)
// }
//
// void* k_list_search(KList* list, b8 (*PFN_filter)(void *value)) {
//     K_FRAME_PUSH_WITH("0x%p, 0x%p", list, PFN_filter)
//     if (list == NULL) {
//         KERROR("KList is NULL")
//         K_FRAME_POP_WITH(NULL)
//     }
//
//     if (PFN_filter == NULL) {
//         KERROR("PFN_filter is null")
//         K_FRAME_POP_WITH(NULL)
//     }
//
//     struct TLNode* node = list->head;
//     while (node != NULL) {
//         if (PFN_filter(node->payload)) {
//             K_FRAME_POP_WITH(node->payload);
//         }
//
//         node = node->next;
//     }
//
//     K_FRAME_POP_WITH(NULL)
// }
//
// void k_list_foreach(KList* list, void (*PFN_handler)(void *value)) {
//     K_FRAME_PUSH_WITH("0x%p, 0x%p", list, PFN_handler)
//     if (list == NULL) {
//         KERROR("KList is NULL")
//         K_FRAME_POP
//     }
//     if (PFN_handler == NULL) {
//         KERROR("PFN_handler is NULL")
//         K_FRAME_POP
//     }
//
//     const struct TLNode* node = list->head;
//     while (node != NULL) {
//         PFN_handler(node->payload);
//         node = node->next;
//     }
//
//     K_FRAME_POP
// }
//
// void k_list_add(KList* list, void *value) {
//     K_FRAME_PUSH_WITH("0x%p, 0x%p", list, value)
//     if (list == NULL) {
//         KERROR("KList is NULL")
//         K_FRAME_POP
//     }
//
//     if (value == NULL) {
//         KERROR("value is NULL")
//         K_FRAME_POP
//     }
//
//     struct TLNode* created = k_list_create_node(list->allocator, value);
//
//     if (list->head == NULL) {
//         list->length++;
//         list->head = created;
//         list->tail = created;
//         K_FRAME_POP
//     }
//
//     if (list->head == list->tail) {
//         list->length++;
//         list->tail = created;
//         list->tail->previous = list->head;
//         list->head->next = list->tail;
//         K_FRAME_POP
//     }
//
//     list->length++;
//     created->previous = list->tail;
//     list->tail->next = created;
//     list->tail = created;
//
//     K_FRAME_POP
// }
//
// void k_list_remove(KList* list, void *value) {
//     K_FRAME_PUSH_WITH("0x%p, 0x%p", list, value)
//     if (list == NULL) {
//         KERROR("KList is NULL")
//         K_FRAME_POP
//     }
//
//     if (list->head == NULL) {
//         KERROR("KList is empty")
//         K_FRAME_POP
//     }
//
//     if (value == NULL) {
//         KERROR("value is NULL")
//         K_FRAME_POP
//     }
//
//     if (list->length == 1) {
//         if (list->head->payload == value) {
//             list->length--;
//             list->head = NULL;
//             list->tail = NULL;
//         } else {
//             KWARN("The KList 0x%p does not contain 0x%p", list, value)
//         }
//
//         K_FRAME_POP
//     }
//
//     if (list->tail->payload == value) {
//         list->length--;
//         list->tail->previous->next = NULL;
//         list->tail = list->tail->previous;
//
//         K_FRAME_POP
//     }
//
//     const struct TLNode* node = list->head;
//     while (node != NULL) {
//         if (node->payload == value) {
//             struct TLNode* previous = node->previous;
//             struct TLNode* next = node->next;
//
//             previous->next = next;
//             next->previous = previous;
//             break;
//         }
//
//         node = node->next;
//     }
//
//     K_FRAME_POP
// }
//
// b8 k_list_after(KList* list, void *item, void *value) {
//     K_FRAME_PUSH_WITH("0x%p, 0x%p, 0x%p", list, item, value)
//     if (list == NULL) {
//         KERROR("KList is NULL")
//         K_FRAME_POP_WITH(false)
//     }
//
//     if (list->head == NULL) {
//         KERROR("KList is empty")
//         K_FRAME_POP_WITH(false)
//     }
//
//     if (item == NULL) {
//         KERROR("item is NULL")
//         K_FRAME_POP_WITH(false)
//     }
//
//     if (value == NULL) {
//         KERROR("value is NULL")
//         K_FRAME_POP_WITH(false)
//     }
//
//     struct TLNode* node = list->head;
//     while (node != NULL) {
//         if (node->payload == item) {
//             struct TLNode* created = k_list_create_node(list->allocator, value);
//
//             created->next = node->next;
//             created->previous = node;
//             node->next = created;
//             K_FRAME_POP_WITH(true)
//         }
//
//         node = node->next;
//     }
//
//     K_FRAME_POP_WITH(false)
// }
//
// b8 k_list_before(KList* list, void *item, void *value) {
//     K_FRAME_PUSH_WITH("0x%p, 0x%p, 0x%p", list, item, value)
//     if (list == NULL) {
//         KERROR("KList is NULL")
//         K_FRAME_POP_WITH(false)
//     }
//
//     if (list->head == NULL) {
//         KERROR("KList is empty")
//         K_FRAME_POP_WITH(false)
//     }
//
//     if (item == NULL) {
//         KERROR("item is NULL")
//         K_FRAME_POP_WITH(false)
//     }
//
//     if (value == NULL) {
//         KERROR("value is NULL")
//         K_FRAME_POP_WITH(false)
//     }
//
//     struct TLNode* node = list->head;
//     while (node != NULL) {
//         if (node->payload == item) {
//             struct TLNode* created = k_list_create_node(list->allocator, value);
//
//             created->next = node;
//             created->previous = node->previous;
//             node->previous->next = created;
//             node->previous = created;
//             K_FRAME_POP_WITH(true)
//         }
//
//         node = node->next;
//     }
//
//     K_FRAME_POP_WITH(false)
// }
//
// b8 k_list_contains(KList* list, void *value) {
//     K_FRAME_PUSH_WITH("0x%p, 0x%p", list, value)
//     if (list == NULL) {
//         KERROR("KList is NULL")
//         K_FRAME_POP_WITH(false)
//     }
//
//     if (list->head == NULL) {
//         KERROR("KList is empty")
//         K_FRAME_POP_WITH(false)
//     }
//
//     if (value == NULL) {
//         KERROR("value is NULL")
//         K_FRAME_POP_WITH(false)
//     }
//
//     struct TLNode* node = list->head;
//     while (node != NULL) {
//         if (node->payload == value) {
//             K_FRAME_POP_WITH(true)
//         }
//
//         node = node->next;
//     }
//
//     K_FRAME_POP_WITH(false)
//
// }
//
// // #####################################################################################################################
// //
// //                                                     ITERATOR
// //
// // #####################################################################################################################
// struct KIterator {u64 capacity; u64 length; void* node; void* (*next)(KIterator*); };
//
// static void* k_list_iterator_next(KIterator *iterator) {
//     K_FRAME_PUSH_WITH("0x%p", iterator)
//     if (iterator == NULL || iterator->node == NULL) {
//         K_FRAME_POP_WITH(NULL)
//     }
//
//     if (iterator->length == iterator->capacity) {
//         K_FRAME_POP_WITH(NULL)
//     }
//
//     struct TLNode* node = iterator->node;
//     void* value = node->payload;
//     iterator->node = node->next;
//     iterator->length++;
//
//     K_FRAME_POP_WITH(value)
// }
//
// KIterator* k_list_iterator_create(KAllocator *allocator, KList* list) {
//     K_FRAME_PUSH_WITH("0x%p", list)
//     KIterator* iterator = k_memory_allocator_alloc(allocator == NULL ? list->allocator : allocator, sizeof(KIterator), K_MEMORY_CONTAINER_ITERATOR);
//     iterator->length = 0;
//     iterator->capacity = list->length;
//     iterator->node = list->head;
//     iterator->next = k_list_iterator_next;
//
//     K_FRAME_POP_WITH(iterator)
// }