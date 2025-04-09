#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"

#include <pthread.h>
#include <asm-generic/errno-base.h>

typedef struct {
    pthread_t handle;
    PFN_task task;
    u64 created_at;
    u64 task_acquired_at;
    u64 id;
} TLThread;

typedef struct {
    TLThread thread[TL_THREAD_POOL_SIZE];
    PFN_task stack[U8_MAX];
    u8 index;

    pthread_mutex_t mutex;
    pthread_cond_t condition;
} TLThreadPool;

static TLThreadPool thread_pool;

static void* tl_thread_runner(void *parameter) {
    TLThread *thread = parameter;
    thread->id = pthread_self();
    KTRACE("Initializing ThreadPoll Thread")

    while (global->application.running) {

        pthread_mutex_lock(&thread_pool.mutex);
        while (thread_pool.index == U8_MAX) {
            pthread_cond_wait(&thread_pool.condition, &thread_pool.mutex);
        }

        thread->task_acquired_at = k_time_epoch_micros();
        thread->task = thread_pool.stack[thread_pool.index];
        KTRACE("Accepting function 0x%p", thread->task)

        thread_pool.stack[thread_pool.index] = NULL;
        thread_pool.index--;

        pthread_mutex_unlock(&thread_pool.mutex);
        thread->task();
    }

    return NULL;
}

b8 tl_thread_initialize(void) {
    K_FRAME_PUSH
    KTRACE("Initializing Threadpool");

    pthread_cond_init(&thread_pool.condition, NULL);
    pthread_mutex_init(&thread_pool.mutex, NULL);
    pthread_mutex_lock(&thread_pool.mutex);
    k_memory_set(thread_pool.stack, 0, TL_ARR_SIZE(thread_pool.stack, u64*));

    for (u8 i = 0 ; i < TL_THREAD_POOL_SIZE ; ++i) {
        thread_pool.thread[i].created_at = k_time_epoch_micros();
        if (pthread_create(&thread_pool.thread[i].handle, NULL, tl_thread_runner, &thread_pool.thread[i]) != 0) {
            KERROR("Failed to create Threadpool thread");
            K_FRAME_POP_WITH(false)
        }
    }

    K_FRAME_POP_WITH(true)
}

b8 tl_thread_terminate(void) {
    K_FRAME_PUSH

    KTRACE("Terminating Threadpool");

    for (u8 i = 0 ; i < TL_THREAD_POOL_SIZE ; ++i) {
        if (thread_pool.thread[i].handle != 0) {
            KTRACE("Cancelling Threadpool thread %llu", thread_pool.thread[i].id);
            const i32 result = pthread_cancel(thread_pool.thread[i].handle);
            if (result != 0) {
                if (result == ESRCH) { KERROR("Failed to cancel thread: no thread with the id %#x could be found.", thread_pool.thread[i].id); }
                else                 { KERROR("Failed to cancel thread: an unknown error has occurred. errno=%i", result); }

                K_FRAME_POP_WITH(false)
            }
        }
    }

    pthread_mutex_destroy(&thread_pool.mutex);
    pthread_cond_destroy(&thread_pool.condition);

    K_FRAME_POP_WITH(true)
}

static u8 tl_thread_submit(PFN_task task) {
    K_FRAME_PUSH_WITH("0x%p", task);

    pthread_mutex_lock(&thread_pool.mutex);
    if (thread_pool.index == U8_MAX - 1) {
        KWARN("ThreadPool is full, rejecting task")
        K_FRAME_POP_WITH(false)
    }

    thread_pool.stack[thread_pool.index] = task;
    thread_pool.index++;
    pthread_mutex_unlock(&thread_pool.mutex);

    pthread_cond_signal(&thread_pool.condition);

    K_FRAME_POP_WITH(true)
}

void tl_thread_fire_and_forget(PFN_task task) {
    K_FRAME_PUSH_WITH("0x%p", task);
    tl_thread_submit(task);
    K_FRAME_POP
}

void tl_thread_fire_and_wait(PFN_task task, const u64 timeout) {
    K_FRAME_PUSH_WITH("0x%p, %llu", task, timeout);
    tl_thread_submit(task);
    K_FRAME_POP
}