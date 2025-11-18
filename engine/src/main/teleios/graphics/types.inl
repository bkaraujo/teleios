#ifndef __TELEIOS_GRAPHICS_TYPES__
#define __TELEIOS_GRAPHICS_TYPES__

#include "teleios/defines.h"

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
        void* (*func_with_args)(void**);  // Changed to accept void** (array of pointers)
    };

    void** args;                        // Array of void* arguments (NULL if NO_ARGS)
    u32 args_count;                     // Number of arguments in the array
    void* result;                       // Result of the processing

    b8 is_sync;                         // Synchronization for sync jobs
    b8 completed;                       // Flag to prevent lost wakeup
    TLMutex* completion_mutex;          // Pre-allocated mutex for sync jobs
    TLCondition* completion_condition;  // Pre-allocated condition for sync jobs
} TLGraphicTask;

static TLQueue* m_queue;
static TLObjectPool* m_task_pool;
static b8 m_shutdown;                   // Shutdown flag for worker thread

struct TLShader {
    u64 handle;
};

#endif