#ifndef __TELEIOS_PLATFORM_TYPES__
#define __TELEIOS_PLATFORM_TYPES__

#include "teleios/defines.h"

/**
 * @brief Platform function pointer table
 *
 * This structure holds function pointers to platform-specific implementations.
 * The platform dispatcher (platform.c) initializes these pointers at compile-time
 * based on the detected operating system.
 *
 * IMPORTANT: Platform selection happens at compile-time using #ifdef guards,
 * not runtime dispatch, because the platform is known at build time.
 * This pattern differs from memory/thread systems which use runtime dispatch.
 */
typedef struct {
    b8          (*initialize            )(void);
    b8          (*terminate             )(void);
    i8          (*fs_path_separator        )(void);
    const char* (*fs_current_directory )(void);
    u64         (*time_epoch_millis     )(void);
    u64         (*time_epoch_micros     )(void);
    void        (*time_clock            )(TLDateTime*);
    
    // Filesystem
    TLString*   (*fs_read               )(const TLString*);
    b8          (*fs_exists             )(const TLString*);
    u64         (*fs_size               )(const TLString*);
} TLPlatform;

#endif
