// #include "libk/libk.h"
//
// // #####################################################################################################################
// //
// //                                                     ITERATOR
// //
// // #####################################################################################################################
// struct KIterator {u64 capacity; u64 length; void* node; void* (*next)(KIterator*); };
//
// // #####################################################################################################################
// //
// //                                                     LIST
// //
// // #####################################################################################################################
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
// };
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
// // #####################################################################################################################
// //
// //                                                     STACK
// //
// // #####################################################################################################################
// KStack* k_stack_create(KAllocator *allocator)  {
//     K_FRAME_PUSH_WITH("0x%p", allocator)
//     KStack* stack = k_memory_allocator_alloc(allocator, sizeof(KStack), K_MEMORY_CONTAINER_STACK);
//     stack = k_list_create(allocator);
//     K_FRAME_POP_WITH(stack)
// }
//
// void k_stack_push(KStack* stack, void* value) {
//     K_FRAME_PUSH_WITH("0x%p, 0x%p", stack, value)
//     k_list_add(stack, value);
//     K_FRAME_POP
// }
//
// void* k_stack_peek(KStack* stack) {
//     K_FRAME_PUSH_WITH("0x%p", stack)
//     void* value = NULL;
//     KIterator* iterator = k_list_iterator_create(NULL, stack);
//     void* next = k_iterator_next(iterator);
//     while (next != NULL) { value = next; next = k_iterator_next(iterator); }
//     K_FRAME_POP_WITH(value)
// }
//
// void* k_stack_pop(KStack* stack) {
//     K_FRAME_PUSH_WITH("0x%p", stack)
//     void* value = k_stack_peek(stack);
//     k_list_remove(stack, value);
//     K_FRAME_POP_WITH(value)
// }
//
// u64 k_stack_length(KStack* stack) {
//     K_FRAME_PUSH_WITH("0x%p", stack)
//     K_FRAME_POP_WITH(k_list_length(stack))
// }
//
// KIterator* k_stack_iterator_create(KStack* stack) {
//     K_FRAME_PUSH_WITH("0x%p", stack)
//     KIterator* iterator = k_memory_allocator_alloc(stack->allocator, sizeof(KIterator), K_MEMORY_CONTAINER_ITERATOR);
//     iterator->length = 0;
//     iterator->capacity = stack->length;
//     iterator->node = stack->head;
//     iterator->next = k_list_iterator_next;
//     K_FRAME_POP_WITH(iterator)
// }