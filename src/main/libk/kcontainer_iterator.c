// #include "libk/libk.h"
//
// struct KIterator {u64 capacity; u64 length; void* node; void* (*next)(KIterator*); };
// void* k_iterator_next(KIterator* iterator) {
//     K_FRAME_PUSH_WITH("0x%p", iterator)
//     if (iterator == NULL || iterator->node == NULL) {
//         K_FRAME_POP_WITH(NULL)
//     }
//
//     void *next = iterator->next(iterator);
//     K_FRAME_POP_WITH(next)
// }
