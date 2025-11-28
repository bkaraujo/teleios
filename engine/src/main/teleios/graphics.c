#include "teleios/teleios.h"
#include "teleios/graphics.h"

static TLQueue* m_queue;
static TLThread* m_thread;
static TLObjectPool* m_pool;

typedef enum {
    TL_TASK_UNKNOWN,
    TL_RETURN_WITH_NO_ARG,
    TL_RETURN_WITH_ARG,
    TL_NORETURN_WITH_NO_ARG,
    TL_NORETURN_WITH_ARG
} TLGraphicsTaskType;

typedef struct {
    TLGraphicsTaskType type;
    union {
        /** @brief Has return with arguments */
        TLFunctionRWA rwa;
        /** @brief Has return without arguments */
        TLFunctionRNA rna;
        /** @brief No return with arguments */
        TLFunctionVWA vwa;
        /** @brief No Return witout arguments */
        TLFunctionVNA vna;

        void* raw;
    } function;

    void* argv;
    void* result;
    TLMutex* mutex;
    TLCondition* condition;
    u8 argc;
    b8 wait;
    b8 is_complete;
} TLGraphicsTask;

#include "teleios/graphics/thread.inl"
#include "teleios/graphics/event.inl"
#define TL_GRAPHICS_QUEUE_SIZE 256

b8 tl_graphics_initialize(void) {
    TL_PROFILER_PUSH

    m_queue = tl_queue_create(global->allocator, TL_GRAPHICS_QUEUE_SIZE, true);
    if (m_queue == NULL) TLFATAL("Failed to create Graphics Queue")

    m_pool = tl_pool_create(global->allocator, sizeof(TLGraphicsTask),TL_GRAPHICS_QUEUE_SIZE, true);
    if (m_pool == NULL) TLFATAL("Failed to create Graphics Task Pool")
    for (u16 i = 0; i < TL_GRAPHICS_QUEUE_SIZE; ++i) {
        TLGraphicsTask* task = tl_pool_acquire(m_pool);
        task->mutex = tl_mutex_create(global->allocator);
        task->condition = tl_condition_create(global->allocator);
        tl_pool_release(m_pool, task);
    }

    tl_event_subscribe(TL_EVENT_WINDOW_RESIZED, tl_graphics_handle_window_resized);

    m_thread = tl_thread_create(global->allocator, tl_graphics_thread, NULL);
    if (m_thread == NULL) TLFATAL("Failed to create Graphics Thread")

    TL_PROFILER_POP_WITH(true)
}

#include "teleios/graphics/queue.inl"

b8 tl_graphics_terminate(void) {
    TL_PROFILER_PUSH

    if (m_thread) {
        tl_thread_join(m_thread, NULL);
        m_thread = NULL;
    }

    if (m_queue) {
        tl_queue_destroy(m_queue);
        m_queue = NULL;
    }

    if (m_pool) {
        for (u16 i = 0; i < TL_GRAPHICS_QUEUE_SIZE; ++i) {
            TLGraphicsTask* task = tl_pool_acquire(m_pool);
            tl_mutex_destroy(task->mutex);
            tl_condition_destroy(task->condition);
            tl_pool_release(m_pool, task);
        }
        tl_pool_destroy(m_pool);

        m_pool = NULL;
    }


    TL_PROFILER_POP_WITH(true)
}