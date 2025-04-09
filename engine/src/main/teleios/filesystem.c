#include "teleios/core.h"
#include "teleios/runtime.h"

TLString * tl_filesystem_get_parent(TLString *path) {
    K_FRAME_PUSH_WITH("0x%p", path)
    if (path == NULL) K_FRAME_POP_WITH(NULL)

    const u32 index = tl_string_last_index_of(path, '/');
    if (index == U32_MAX) K_FRAME_POP_WITH(NULL)


    TL_CREATE_CHAR(value, index + 1)
    tl_char_copy(value, tl_string(path), index);
    TLString *parent = tl_string_clone(tl_string_arena(path), value);

    K_FRAME_POP_WITH(parent)
}