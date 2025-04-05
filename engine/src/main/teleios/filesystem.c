#include "teleios/core.h"
#include "teleios/runtime.h"

TLString * tl_filesystem_get_parent(TLString *path) {
    TL_STACK_PUSHA("0x%p", path)
    if (path == NULL) TL_STACK_POPV(NULL)

    const u32 index = tl_string_last_index_of(path, '/');
    if (index == U32_MAX) TL_STACK_POPV(NULL)


    TL_CREATE_CHAR(value, index + 1)
    tl_char_copy(value, tl_string(path), index);
    TLString *parent = tl_string_clone(tl_string_arena(path), value);

    TL_STACK_POPV(parent)
}