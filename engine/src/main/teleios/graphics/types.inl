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
    union {
        void* (*func_no_args)(void);
        void* (*func_with_args)(void**);  // Changed to accept void** (array of pointers)
    };

    void** args;                        // Array of void* arguments (NULL if NO_ARGS)
    void* result;                       // Result of the processing
    TLMutex* completion_mutex;          // Pre-allocated mutex for sync jobs
    TLCondition* completion_condition;  // Pre-allocated condition for sync jobs
    u32 args_count;                     // Number of arguments in the array
    TLGraphicsJobType type;
    b8 is_sync;                         // Synchronization for sync jobs
    b8 completed;                       // Flag to prevent lost wakeup
} TLGraphicTask;

static TLQueue* m_queue;
static TLObjectPool* m_task_pool;
static b8 m_shutdown;                   // Shutdown flag for worker thread

struct TLShader {
    u64 handle;
};

// ---------------------------------
// Command Buffer Definitions
// ---------------------------------

typedef enum {
    TL_CMDBUFFER_FREE,       // Available for recording
    TL_CMDBUFFER_RECORDING,  // Main thread is writing commands
    TL_CMDBUFFER_PENDING,    // Submitted, waiting for graphics thread
    TL_CMDBUFFER_EXECUTING   // Graphics thread is processing
} TLCommandBufferState;

typedef struct {
    union {
        void* (*func_no_args)(void);
        void* (*func_with_args)(void**);
    };
    void** args;
    u32 args_count;
    TLGraphicsJobType type;
} TLCommand;

#define TL_CMDBUFFER_CAPACITY 256

typedef struct {
    TLCommand commands[TL_CMDBUFFER_CAPACITY];
    u32 count;
    TLCommandBufferState state;
} TLCommandBuffer;

#define TL_CMDBUFFER_POOL_SIZE 3

typedef struct {
    TLCommandBuffer buffers[TL_CMDBUFFER_POOL_SIZE];
    TLMutex* mutex;
    TLCondition* buffer_available;  // Signaled when a buffer becomes FREE
    TLCondition* buffer_pending;    // Signaled when a buffer becomes PENDING
    TLAllocator* allocator;
    u8 recording_index;             // Index of buffer being recorded (or 0xFF if none)
} TLCommandBufferPool;

#endif