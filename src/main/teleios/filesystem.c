#include "teleios/teleios.h"

KString * tl_filesystem_get_parent(KString *path) {
    K_FRAME_PUSH_WITH("0x%p", path)
    if (path == NULL)
        K_FRAME_POP_WITH(NULL)

    const u32 index = k_string_last_index_of(path, K_PATH_SEPARATOR);
    if (index == U32_MAX)
        K_FRAME_POP_WITH(NULL)

    k_memory_alloc_stack(value, index + 1);
    k_char_copy(value, k_string(path), index);
    KString *parent = k_string_clone(k_string_allocator(path), value);
    k_memory_free_stack(value);

    K_FRAME_POP_WITH(parent)
}