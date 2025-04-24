#include "libk/libk.h"

typedef struct BKMemory {
    void *pointer;
    struct BKMemory *next;
} BKMemory;

static BKMemory head;

void* k_memory_alloc(const u64 size) {
    K_FRAME_PUSH_WITH("%ull", size)
    // =======================================================================
    // Find the spot to create the BKMemory
    // =======================================================================
    BKMemory *node = NULL;
    if (head.next == NULL) { node = &head; }
    else {
        BKMemory *current = head.next;
        while (current->next != NULL) current = current->next;
        node = current;
    }
    // =======================================================================
    // Allocate the memory a record stack frame of the caller
    // =======================================================================
    node->next = malloc(sizeof(BKMemory));
    memset(node->next, 0, sizeof(BKMemory));

    node->next->pointer = malloc(size);
    if (node->next->pointer == NULL) KFATAL("Failed to allocate %llu bytes", size)
    // =======================================================================
    // Deliver the result
    // =======================================================================
    KTRACE("Allocated 0x%p with %llu bytes", node->next->pointer, size)
    K_FRAME_POP_WITH(node->next->pointer)
}

void k_memory_free(void *block) {
    K_FRAME_PUSH_WITH("0x%p", block)

    if (block == NULL) KFATAL("block is NULL")
    if (head.next == NULL) KFATAL("Unknown block 0x%p", block)
    // =======================================================================
    // Release the BKMemory from the list
    // =======================================================================
    BKMemory *node = NULL;
    if (head.next->pointer == block) {
        node = head.next;
        head.next = node->next;
    } else {
        BKMemory *s = head.next;
        while (s != NULL) {
            BKMemory *current = s->next;
            if (current == NULL) break;
            if (current->pointer == block) {
                s->next = current->next;
                node = current;
                break;
            }

            s = current;
        }
    }

    if (node == NULL) KFATAL("Unknown block 0x%p", block)
    // =======================================================================
    // Release the memory
    // =======================================================================
    free(node->pointer);
    free(node);

    KTRACE("Deallocated 0x%p", block)
    K_FRAME_POP
}

void k_memory_set(void *target, const i32 value, const u64 size) {
    K_FRAME_PUSH_WITH("0x%p, %d, llu", target, value, size)

    if (target == NULL) KFATAL("target is NULL")
    if (size == 0) KFATAL("size is 0")
    memset(target, value, size);

    K_FRAME_POP
}

void k_memory_copy(void *target, const void *source, const u64 size) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p, %llu", target, source, size)
    memcpy(target, source, size);
    K_FRAME_POP
}
