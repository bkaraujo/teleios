#ifndef __TELEIOS_GRAPHICS_TYPES__
#define __TELEIOS_GRAPHICS_TYPES__

#include "teleios\teleios.h"
#include <glad/glad.h>

// ---------------------------------
// Job Type Definitions
// ---------------------------------

typedef enum {
    TL_GRAPHICS_JOB_NO_ARGS,
    TL_GRAPHICS_JOB_WITH_ARGS
} TLGraphicsJobType;

typedef struct {
    TLGraphicsJobType type;

    union {
        void (*func_no_args)(void);
        void (*func_with_args)(void*);
    };

    void* args;  // NULL if NO_ARGS

    // Synchronization for sync jobs
    b8 is_sync;
    b8 completed;
    TLMutex* completion_mutex;
    TLCondition* completion_condition;
} TLGraphicTask;

// ---------------------------------
// Graphics Queue
// ---------------------------------

typedef struct {
    TLGraphicTask* tasks;
    u32 capacity;
    u32 head;               // Next slot for insertion
    u32 tail;               // Next slot for removal
    u32 count;

    TLMutex* mutex;
    TLCondition* not_empty;  // Signals worker when work is available
    TLCondition* not_full;   // Signals producer when space is available

    b8 shutdown;  // Flag to terminate worker thread
} TLGraphicsQueue;

#endif