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
//
// // #####################################################################################################################
// //
// //                                                     MAP
// //
// // #####################################################################################################################
// struct KMapEntry { KString *key; void *payload; };
// struct KMap { KAllocator *allocator; struct KMapEntry *values; u16 size; u16 length; };
//
// KMap* k_map_create(KAllocator *allocator) {
//     K_FRAME_PUSH_WITH("0x%p", allocator)
//     if (allocator == NULL) KFATAL("allocator is NULL")
//
//     KMap *map = k_memory_allocator_alloc(allocator, sizeof(KMap), K_MEMORY_CONTAINER_MAP);
//     map->size = 10;
//     map->length = 0;
//     map->allocator = allocator;
//
//     K_FRAME_POP_WITH(map)
// }
//
// void k_map_put(KMap* map, const char *key, void *value) {
//     K_FRAME_PUSH_WITH("0x%p, %s, 0x%p", map, key, value)
//     if (map == NULL) KFATAL("KMap is NULL")
//     if (key == NULL) KFATAL("key is NULL")
//     if (value == NULL) KFATAL("value is NULL")
//
//     if (map->values == NULL) {
//         KTRACE("KMap 0x%p initialized with capacity of %d", map, map->size)
//         map->values = k_memory_allocator_alloc(map->allocator, map->size * sizeof(struct KMapEntry), K_MEMORY_CONTAINER_NODE);
//     }
//
//     if (map->length >= map->size) {
//         u16 new_size = (u16)((f32)map->size * 1.75f) + 1;
//         KTRACE("KMap 0x%p resized from %d to %d capacity", map, map->size, new_size)
//         void *new_values = k_memory_allocator_alloc(map->allocator, new_size * sizeof(struct KMapEntry), K_MEMORY_CONTAINER_NODE);
//         k_memory_copy(new_values, map->values, map->size * sizeof(struct KMapEntry));
//         map->size = new_size;
//         map->values = new_values;
//     }
//
//     for (u16 i = 0; i < map->length; i++) {
//         struct KMapEntry *entry = &map->values[i];
//         if (entry->key == NULL) continue;
//         if (k_string_equals(entry->key, key)) {
//             entry->payload = value;
//             K_FRAME_POP
//         }
//     }
//
//     map->values[map->length].key = k_string_clone(map->allocator, key);
//     map->values[map->length].payload = value;
//     map->length++;
//     K_FRAME_POP
// }
//
// void* k_map_get(KMap* map, const char *key) {
//     K_FRAME_PUSH_WITH("0x%p, %", map, key)
//     if (map == NULL) KFATAL("KMap is NULL")
//     if (key == NULL) KFATAL("key is NULL")
//
//     for (u16 i = 0; i < map->size; i++) {
//         if (k_string_equals(map->values[i].key, key)) {
//             K_FRAME_POP_WITH(map->values[i].payload)
//         }
//     }
//
//     K_FRAME_POP_WITH(NULL)
// }
//
// u16 k_map_length(KMap* map) {
//     K_FRAME_PUSH_WITH("0x%p", map)
//     const u16 length = map->length;
//     K_FRAME_POP_WITH(length)
// }
//
// b8 k_map_contains(KMap* map, const char *key) {
//     K_FRAME_PUSH_WITH("0x%p, %", map, key)
//     b8 contains = k_map_get(map, key) != NULL;
//     K_FRAME_POP_WITH(contains)
// }
//
// void k_map_remove(KMap* map, const char *key) {
//     K_FRAME_PUSH_WITH("0x%p, %", map, key)
//     if (map == NULL) KFATAL("KMap is NULL")
//     if (key == NULL) KFATAL("key is NULL")
//
//     for (u16 i = 0; i < map->size; i++) {
//         if (k_string_equals(map->values[i].key, key)) {
//             k_memory_copy(
//                 map->values + i,
//                 map->values + i + 1,
//                 sizeof(struct KMapEntry) * (map->size - map->length - 1)
//             );
//
//             map->length--;
//             k_memory_set(
//                 map->values + map->length,
//                 0,
//                 sizeof(struct KMapEntry) * (map->size - map->length)
//             );
//         }
//     }
//
//     K_FRAME_POP
// }
//
// KIterator* k_map_keys(KAllocator *allocator, KMap* map) {
//     K_FRAME_PUSH_WITH("0x%p", map)
//     if (map == NULL) KFATAL("KMap is NULL")
//
//     KList *keys = k_list_create(map->allocator);
//     if (map->length == 0) K_FRAME_POP_WITH(NULL)
//
//     for (u16 i = 0; i < map->length; ++i) {
//         k_list_add(keys, map->values[i].key);
//     }
//
//     KIterator *it = k_list_iterator_create(allocator, keys);
//     K_FRAME_POP_WITH(it)
// }