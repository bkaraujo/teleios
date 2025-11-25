#ifndef __TELEIOS_THREAD_CONDITION__
#define __TELEIOS_THREAD_CONDITION__
#include "teleios/teleios.h"
#include "teleios/thread/types.inl"

TLCondition* tl_condition_create(TLAllocator* allocator) {
    TL_PROFILER_PUSH_WITH("0x%p", allocator)
    if (allocator == NULL) {
        TLWARN("Attempted to use a NULL TLAllocator")
        TL_PROFILER_POP_WITH(NULL)
    }

    TLCondition* condition = (TLCondition*)tl_memory_alloc(allocator, TL_MEMORY_THREAD, sizeof(TLCondition));
    condition->allocator = allocator;

#if defined(TL_PLATFORM_UNIX)
    i32 result = pthread_cond_init(&condition->cond, NULL);
    if (result != 0) {
        TLERROR("tl_condition_create: pthread_cond_init failed with error %d", result);
        tl_memory_free(condition->allocator, condition);
        TL_PROFILER_POP_WITH(NULL)
    }
#elif defined(TL_PLATFORM_WINDOWS)
    InitializeConditionVariable(&condition->cv);
#endif

    TLTRACE("Condition variable created:0x%p", condition);
    TL_PROFILER_POP_WITH(condition);
}

void tl_condition_destroy(TLCondition* condition) {
    TL_PROFILER_PUSH_WITH("0x%p", condition)
    if (!condition) {
        TLERROR("tl_condition_destroy: condition cannot be NULL");
        TL_PROFILER_POP
    }

#if defined(TL_PLATFORM_UNIX)
    i32 result = pthread_cond_destroy(&condition->cond);
    if (result != 0) {
        TLERROR("tl_condition_destroy: pthread_cond_destroy failed with error %d", result);
    }
#elif defined(TL_PLATFORM_WINDOWS)
    // Windows condition variables don't need explicit cleanup
    TLTRACE("Condition variable destroyed:0x%p", condition);
#endif

    tl_memory_free(condition->allocator, condition);
    TL_PROFILER_POP
}

b8 tl_condition_wait(TLCondition* condition, TLMutex* mutex) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", condition, mutex)
    if (!condition) {
        TLERROR("tl_condition_wait: condition cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }
    if (!mutex) {
        TLERROR("tl_condition_wait: mutex cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }

#if defined(TL_PLATFORM_UNIX)
    i32 result = pthread_cond_wait(&condition->cond, &mutex->mutex);
    if (result != 0) {
        TLERROR("tl_condition_wait: pthread_cond_wait failed with error %d", result);
        TL_PROFILER_POP_WITH(false)
    }
#elif defined(TL_PLATFORM_WINDOWS)
    const BOOL result = SleepConditionVariableCS(&condition->cv, &mutex->cs, INFINITE);
    if (!result) {
        TLERROR("tl_condition_wait: SleepConditionVariableCS failed with error %lu", GetLastError());
        TL_PROFILER_POP_WITH(false)
    }
#endif


    TL_PROFILER_POP_WITH(true)
}

b8 tl_condition_wait_timeout(TLCondition* condition, TLMutex* mutex, const u32 timeout_ms) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, %u", condition, mutex, timeout_ms)

    if (!condition) {
        TLERROR("tl_condition_wait_timeout: condition cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }
    if (!mutex) {
        TLERROR("tl_condition_wait_timeout: mutex cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }

#if defined(TL_PLATFORM_UNIX)
    // Calculate absolute timeout
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    // Add milliseconds to timespec
    u64 nsec = ts.tv_nsec + (timeout_ms * 1000000ULL);
    ts.tv_sec += nsec / 1000000000ULL;
    ts.tv_nsec = nsec % 1000000000ULL;

    i32 result = pthread_cond_timedwait(&condition->cond, &mutex->mutex, &ts);
    if (result == ETIMEDOUT) {
        TL_PROFILER_POP_WITH(false)  // Timeout, not an error
    }
    if (result != 0) {
        TLERROR("tl_condition_wait_timeout: pthread_cond_timedwait failed with error %d", result);
        TL_PROFILER_POP_WITH(false)
    }
#elif defined(TL_PLATFORM_WINDOWS)
    const b8 result = (b8)SleepConditionVariableCS(&condition->cv, &mutex->cs, timeout_ms);
    if (!result) {
        const u32 error = (u32)GetLastError();
        if (error == ERROR_TIMEOUT) {
            TL_PROFILER_POP_WITH(false)
        }
        TLERROR("tl_condition_wait_timeout: SleepConditionVariableCS failed with error %u", error);
        TL_PROFILER_POP_WITH(false)
    }
#endif


    TL_PROFILER_POP_WITH(true)
}

b8 tl_condition_signal(TLCondition* condition) {
    TL_PROFILER_PUSH_WITH("0x%p", condition)
    if (!condition) {
        TLERROR("tl_condition_signal: condition cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }

#if defined(TL_PLATFORM_UNIX)
    i32 result = pthread_cond_signal(&condition->cond);
    if (result != 0) {
        TLERROR("tl_condition_signal: pthread_cond_signal failed with error %d", result);
        TL_PROFILER_POP_WITH(false)
    }
#elif defined(TL_PLATFORM_WINDOWS)
    WakeConditionVariable(&condition->cv);
#endif

    TL_PROFILER_POP_WITH(true)
}

b8 tl_condition_broadcast(TLCondition* condition) {
    TL_PROFILER_PUSH_WITH("0x%p", condition)
    if (!condition) {
        TLERROR("tl_condition_broadcast: condition cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }

#if defined(TL_PLATFORM_UNIX)
    i32 result = pthread_cond_broadcast(&condition->cond);
    if (result != 0) {
        TLERROR("tl_condition_broadcast: pthread_cond_broadcast failed with error %d", result);
        TL_PROFILER_POP_WITH(false)
    }
#elif defined(TL_PLATFORM_WINDOWS)
    WakeAllConditionVariable(&condition->cv);
#endif

    TL_PROFILER_POP_WITH(true)
}
#endif