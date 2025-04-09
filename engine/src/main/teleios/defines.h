#ifndef __TELEIOS_DEFINES__
#define __TELEIOS_DEFINES__

#include "libk/libk.h"
#include "cglm/types-struct.h"

#define TL_ARR_LENGTH(a,t) (sizeof(a) / sizeof(t))
#define TL_ARR_SIZE(a,t) ((sizeof(a) / sizeof(t)) * sizeof(t))
#define TL_CREATE_CHAR(name, size) char name[size]; tl_memory_set(name, 0 , size);

// ---------------------------------
// Globals
// ---------------------------------
typedef struct TLMemoryArena TLMemoryArena;
typedef struct TLString TLString;

typedef struct TLList TLStack;
typedef struct TLList TLList;
/**
 * @brief The map hold a <TLString, void*> structure. The structure lives
 * for as long as there a elements in the TLList
 */
typedef struct TLMap TLMap;
typedef struct TLIterator TLIterator;

typedef struct {
    u8 last[16];
    u64 timestamp;
    i32 flags;
    u8 i, j;
    u8 s[256];
} TLUlidGenerator;

typedef struct {
    char text[27];
} TLUlid;

#define TL_SCENE_MAX_ACTORS         5
#define TL_SCENE_MAX_ACTOR_SCRIPTS  10
#define TL_YAML_PROPERTY_MAX_SIZE   1024
#define TL_THREAD_POOL_SIZE         2

#endif // __TELEIOS_DEFINES__