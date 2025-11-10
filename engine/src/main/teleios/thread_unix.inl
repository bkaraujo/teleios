/**
 * @file thread_unix.inl
 * @brief Unix/POSIX-specific threading implementation
 *
 * Uses POSIX threads (pthreads): pthread_create, pthread_mutex, pthread_cond
 * This file is included directly into thread.c
 *
 * Memory management: Uses DYNAMIC allocator for thread resources to allow
 * individual deallocation and memory leak tracking.
 */

#if defined(TL_PLATFORM_LINUX) || defined(TL_PLATFORM_APPLE) || defined(TL_PLATFORM_UNIX)

#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

// ---------------------------------
// Memory allocator for thread resources
// ---------------------------------

static TLAllocator* m_thread_allocator = NULL;

static void tl_thread_ensure_allocator(void) {
    if (m_thread_allocator == NULL) {
        m_thread_allocator = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
        if (!m_thread_allocator) {
            TLFATAL("Failed to create DYNAMIC allocator for thread system");
        }
        TLINFO("Thread system DYNAMIC allocator initialized");
    }
}

// ---------------------------------
// Internal structures
// ---------------------------------

struct TLThread {
    pthread_t handle;
    TLThreadFunc func;
    void* arg;
    void* result;
    b8 detached;
};

struct TLMutex {
    pthread_mutex_t mutex;
};

struct TLCondition {
    pthread_cond_t cond;
};

// ---------------------------------
// Thread wrapper for pthread calling convention
// ---------------------------------

static void* thread_wrapper(void* param) {
    TLThread* thread = (TLThread*)param;
    thread->result = thread->func(thread->arg);
    return thread->result;
}

// ---------------------------------
// Thread Management
// ---------------------------------

TLThread* tl_thread_create(TLThreadFunc func, void* arg) {
    if (!func) {
        TLERROR("tl_thread_create: func cannot be NULL");
        return NULL;
    }

    tl_thread_ensure_allocator();

    TLThread* thread = (TLThread*)tl_memory_alloc(m_thread_allocator, TL_MEMORY_THREAD, sizeof(TLThread));
    if (!thread) {
        TLERROR("tl_thread_create: Failed to allocate thread structure");
        return NULL;
    }

    thread->func = func;
    thread->arg = arg;
    thread->result = NULL;
    thread->detached = false;

    i32 result = pthread_create(&thread->handle, NULL, thread_wrapper, thread);
    if (result != 0) {
        TLERROR("tl_thread_create: pthread_create failed with error %d", result);
        tl_memory_free(m_thread_allocator, thread);
        return NULL;
    }

    TLDEBUG("Thread created: ID=%llu", (u64)thread->handle);
    return thread;
}

b8 tl_thread_join(TLThread* thread, void** result) {
    if (!thread) {
        TLERROR("tl_thread_join: thread cannot be NULL");
        return false;
    }

    if (thread->detached) {
        TLERROR("tl_thread_join: Cannot join a detached thread");
        return false;
    }

    void* thread_result;
    i32 join_result = pthread_join(thread->handle, &thread_result);
    if (join_result != 0) {
        TLERROR("tl_thread_join: pthread_join failed with error %d", join_result);
        return false;
    }

    if (result) {
        *result = thread->result;
    }

    TLDEBUG("Thread joined: ID=%llu", (u64)thread->handle);
    tl_memory_free(m_thread_allocator, thread);
    return true;
}

b8 tl_thread_detach(TLThread* thread) {
    if (!thread) {
        TLERROR("tl_thread_detach: thread cannot be NULL");
        return false;
    }

    if (thread->detached) {
        TLERROR("tl_thread_detach: Thread already detached");
        return false;
    }

    i32 result = pthread_detach(thread->handle);
    if (result != 0) {
        TLERROR("tl_thread_detach: pthread_detach failed with error %d", result);
        return false;
    }

    thread->detached = true;
    TLDEBUG("Thread detached: ID=%llu", (u64)thread->handle);
    tl_memory_free(m_thread_allocator, thread);
    return true;
}

u64 tl_thread_id(void) {
    return (u64)pthread_self();
}

void tl_thread_sleep(u32 milliseconds) {
    usleep(milliseconds * 1000);  // usleep takes microseconds
}

// ---------------------------------
// Mutex (Mutual Exclusion)
// ---------------------------------

TLMutex* tl_mutex_create(TLAllocator* allocator) {
    TLMutex* mutex = (TLMutex*)tl_memory_alloc(allocator, TL_MEMORY_THREAD, sizeof(TLMutex));
    if (!mutex) {
        TLERROR("tl_mutex_create: Failed to allocate mutex structure");
        return NULL;
    }

    i32 result = pthread_mutex_init(&mutex->mutex, NULL);
    if (result != 0) {
        TLERROR("tl_mutex_create: pthread_mutex_init failed with error %d", result);
        tl_memory_free(m_thread_allocator, mutex);
        return NULL;
    }

    TLDEBUG("Mutex created: %p", mutex);
    return mutex;
}

void tl_mutex_destroy(TLMutex* mutex) {
    if (!mutex) {
        TLERROR("tl_mutex_destroy: mutex cannot be NULL");
        return;
    }

    i32 result = pthread_mutex_destroy(&mutex->mutex);
    if (result != 0) {
        TLERROR("tl_mutex_destroy: pthread_mutex_destroy failed with error %d", result);
    }

    TLDEBUG("Mutex destroyed: %p", mutex);
    tl_memory_free(m_thread_allocator, mutex);
}

b8 tl_mutex_lock(TLMutex* mutex) {
    if (!mutex) {
        TLERROR("tl_mutex_lock: mutex cannot be NULL");
        return false;
    }

    i32 result = pthread_mutex_lock(&mutex->mutex);
    if (result != 0) {
        TLERROR("tl_mutex_lock: pthread_mutex_lock failed with error %d", result);
        return false;
    }

    return true;
}

b8 tl_mutex_trylock(TLMutex* mutex) {
    if (!mutex) {
        TLERROR("tl_mutex_trylock: mutex cannot be NULL");
        return false;
    }

    i32 result = pthread_mutex_trylock(&mutex->mutex);
    if (result == EBUSY) {
        return false;  // Mutex already locked, not an error
    }
    if (result != 0) {
        TLERROR("tl_mutex_trylock: pthread_mutex_trylock failed with error %d", result);
        return false;
    }

    return true;
}

b8 tl_mutex_unlock(TLMutex* mutex) {
    if (!mutex) {
        TLERROR("tl_mutex_unlock: mutex cannot be NULL");
        return false;
    }

    i32 result = pthread_mutex_unlock(&mutex->mutex);
    if (result != 0) {
        TLERROR("tl_mutex_unlock: pthread_mutex_unlock failed with error %d", result);
        return false;
    }

    return true;
}

// ---------------------------------
// Condition Variables
// ---------------------------------

TLCondition* tl_condition_create(TLAllocator* allocator) {
    TLCondition* condition = (TLCondition*)tl_memory_alloc(allocator, TL_MEMORY_THREAD, sizeof(TLCondition));
    if (!condition) {
        TLERROR("tl_condition_create: Failed to allocate condition structure");
        return NULL;
    }

    i32 result = pthread_cond_init(&condition->cond, NULL);
    if (result != 0) {
        TLERROR("tl_condition_create: pthread_cond_init failed with error %d", result);
        tl_memory_free(m_thread_allocator, condition);
        return NULL;
    }

    TLDEBUG("Condition variable created: %p", condition);
    return condition;
}

void tl_condition_destroy(TLCondition* condition) {
    if (!condition) {
        TLERROR("tl_condition_destroy: condition cannot be NULL");
        return;
    }

    i32 result = pthread_cond_destroy(&condition->cond);
    if (result != 0) {
        TLERROR("tl_condition_destroy: pthread_cond_destroy failed with error %d", result);
    }

    TLDEBUG("Condition variable destroyed: %p", condition);
    tl_memory_free(m_thread_allocator, condition);
}

b8 tl_condition_wait(TLCondition* condition, TLMutex* mutex) {
    if (!condition) {
        TLERROR("tl_condition_wait: condition cannot be NULL");
        return false;
    }
    if (!mutex) {
        TLERROR("tl_condition_wait: mutex cannot be NULL");
        return false;
    }

    i32 result = pthread_cond_wait(&condition->cond, &mutex->mutex);
    if (result != 0) {
        TLERROR("tl_condition_wait: pthread_cond_wait failed with error %d", result);
        return false;
    }

    return true;
}

b8 tl_condition_wait_timeout(TLCondition* condition, TLMutex* mutex, u32 timeout_ms) {
    if (!condition) {
        TLERROR("tl_condition_wait_timeout: condition cannot be NULL");
        return false;
    }
    if (!mutex) {
        TLERROR("tl_condition_wait_timeout: mutex cannot be NULL");
        return false;
    }

    // Calculate absolute timeout
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    // Add milliseconds to timespec
    u64 nsec = ts.tv_nsec + (timeout_ms * 1000000ULL);
    ts.tv_sec += nsec / 1000000000ULL;
    ts.tv_nsec = nsec % 1000000000ULL;

    i32 result = pthread_cond_timedwait(&condition->cond, &mutex->mutex, &ts);
    if (result == ETIMEDOUT) {
        return false;  // Timeout, not an error
    }
    if (result != 0) {
        TLERROR("tl_condition_wait_timeout: pthread_cond_timedwait failed with error %d", result);
        return false;
    }

    return true;
}

b8 tl_condition_signal(TLCondition* condition) {
    if (!condition) {
        TLERROR("tl_condition_signal: condition cannot be NULL");
        return false;
    }

    i32 result = pthread_cond_signal(&condition->cond);
    if (result != 0) {
        TLERROR("tl_condition_signal: pthread_cond_signal failed with error %d", result);
        return false;
    }

    return true;
}

b8 tl_condition_broadcast(TLCondition* condition) {
    if (!condition) {
        TLERROR("tl_condition_broadcast: condition cannot be NULL");
        return false;
    }

    i32 result = pthread_cond_broadcast(&condition->cond);
    if (result != 0) {
        TLERROR("tl_condition_broadcast: pthread_cond_broadcast failed with error %d", result);
        return false;
    }

    return true;
}

// ---------------------------------
// Thread system cleanup (optional)
// ---------------------------------

/**
 * @brief Cleanup thread system and report memory leaks
 * @note Optional - called during application shutdown to report thread resource leaks
 * @note Not exposed in public API, can be called from platform.c shutdown
 */
static void tl_thread_system_terminate(void) {
    if (m_thread_allocator != NULL) {
        TLINFO("Destroying thread system allocator and checking for leaks...");
        tl_memory_allocator_destroy(m_thread_allocator);
        m_thread_allocator = NULL;
    }
}

#endif // TL_PLATFORM_LINUX || TL_PLATFORM_APPLE || TL_PLATFORM_UNIX
