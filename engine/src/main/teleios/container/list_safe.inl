#ifndef __TELEIOS_CONTAINER_LIST_SAFE__
#define __TELEIOS_CONTAINER_LIST_SAFE__

#include "teleios/teleios.h"
#include "teleios/container/list_unsafe.inl"

void tl_list_safe_push_front(TLList* list, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, data)
    tl_mutex_lock(list->mutex);
    tl_list_unsafe_push_front(list, data);
    tl_mutex_unlock(list->mutex);
    TL_PROFILER_POP
}

void tl_list_safe_push_back(TLList* list, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, data)
    tl_mutex_lock(list->mutex);
    tl_list_unsafe_push_back(list, data);
    tl_mutex_unlock(list->mutex);
    TL_PROFILER_POP
}

void tl_list_safe_insert_after(TLList* list, TLListNode* node, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", list, node, data)
    tl_mutex_lock(list->mutex);
    tl_list_unsafe_insert_after(list, node, data);
    tl_mutex_unlock(list->mutex);
    TL_PROFILER_POP
}

void tl_list_safe_insert_before(TLList* list, TLListNode* node, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", list, node, data)
    tl_mutex_lock(list->mutex);
    tl_list_unsafe_insert_before(list, node, data);
    tl_mutex_unlock(list->mutex);
    TL_PROFILER_POP
}

void* tl_list_safe_pop_front(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    tl_mutex_lock(list->mutex);
    void* result = tl_list_unsafe_pop_front(list);
    tl_mutex_unlock(list->mutex);
    TL_PROFILER_POP_WITH(result)
}

void* tl_list_safe_pop_back(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    tl_mutex_lock(list->mutex);
    void* result = tl_list_unsafe_pop_back(list);
    tl_mutex_unlock(list->mutex);
    TL_PROFILER_POP_WITH(result)
}

void* tl_list_safe_remove(TLList* list, TLListNode* node) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, node)
    tl_mutex_lock(list->mutex);
    void* result = tl_list_unsafe_remove(list, node);
    tl_mutex_unlock(list->mutex);
    TL_PROFILER_POP_WITH(result)
}

void* tl_list_safe_front(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    tl_mutex_lock(list->mutex);
    void* result = tl_list_unsafe_front(list);
    tl_mutex_unlock(list->mutex);
    TL_PROFILER_POP_WITH(result)
}

void* tl_list_safe_back(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    tl_mutex_lock(list->mutex);
    void* result = tl_list_unsafe_back(list);
    tl_mutex_unlock(list->mutex);
    TL_PROFILER_POP_WITH(result)
}

u32 tl_list_safe_size(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    tl_mutex_lock(list->mutex);
    const u32 result = tl_list_unsafe_size(list);
    tl_mutex_unlock(list->mutex);
    TL_PROFILER_POP_WITH(result)
}

b8 tl_list_safe_is_empty(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    tl_mutex_lock(list->mutex);
    const b8 result = tl_list_unsafe_is_empty(list);
    tl_mutex_unlock(list->mutex);
    TL_PROFILER_POP_WITH(result)
}

void tl_list_safe_clear(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    tl_mutex_lock(list->mutex);
    tl_list_unsafe_clear(list);
    tl_mutex_unlock(list->mutex);
    TL_PROFILER_POP
}

#endif
