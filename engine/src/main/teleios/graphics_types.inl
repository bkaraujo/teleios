#ifndef __TELEIOS_GRAPHICS_TYPES__
#define __TELEIOS_GRAPHICS_TYPES__

#include "teleios/teleios.h"

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
        void* (*func_no_args)(void);
        void* (*func_with_args)(void*);
    };

    void* args;                 // NULL if NO_ARGS
    void* result;

    b8 is_sync;                 // Synchronization for sync jobs
    b8 completed;               // Flag to prevent lost wakeup
    b8 heap_allocated;          // True for async jobs (allocated on heap)
    TLMutex* completion_mutex;
    TLCondition* completion_condition;
} TLGraphicTask;

static TLQueue* m_queue;

#endif