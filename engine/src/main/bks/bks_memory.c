#include "bks/bks.h"

BKS_INLINE void* bks_memory_alloc(const u64 size) {
    BKS_TRACE_PUSHA("%ull", size)

    void *block = __builtin_malloc(size);
    if (block == NULL) BKSFATAL("Failed to allocate %llu bytes", size)

    BKS_TRACE_POPV(block)
}

BKS_INLINE void bks_memory_free(void *block) {
    BKS_TRACE_PUSHA("0x%p", block)

    if (block == NULL) BKSFATAL("block is NULL")
    __builtin_free(block);

    BKS_TRACE_POP
}

BKS_INLINE void bks_memory_set(void *target, const i32 value, const u64 size) {
    BKS_TRACE_PUSHA("0x%p, %d, llu", target, value, size)

    if (target == NULL) BKSFATAL("target is NULL")
    if (size == 0) BKSFATAL("size is 0")
    __builtin_memset(target, value, size);

    BKS_TRACE_POP
}

BKS_INLINE void bks_memory_copy(void *target, const void *source, const u64 size) {
    BKS_TRACE_PUSHA("0x%p, 0x%p, %llu", target, source, size)
    __builtin_memcpy(target, source, size);
    BKS_TRACE_POP
}