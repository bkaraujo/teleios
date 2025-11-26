#ifndef __TELEIOS_THREAD_MUTEX__
#define __TELEIOS_THREAD_MUTEX__
#include "teleios/teleios.h"
#include "teleios/thread/types.inl"

TLMutex* tl_mutex_create(TLAllocator* allocator) {
    TL_PROFILER_PUSH_WITH("0x%p", allocator)
    if (allocator == NULL) {
        TLWARN("Attempted to use a NULL TLAllocator")
        TL_PROFILER_POP_WITH(NULL)
    }

    TLMutex* mutex = (TLMutex*)tl_memory_alloc(allocator, TL_MEMORY_THREAD, sizeof(TLMutex));
    mutex->allocator = allocator;

#if defined(TL_PLATFORM_UNIX)
    i32 result = pthread_mutex_init(&mutex->mutex, NULL);
    if (result != 0) {
        TLERROR("tl_mutex_create: pthread_mutex_init failed with error %d", result);
        tl_memory_free(mutex->allocator, mutex);
        return NULL;
    }
#elif defined(TL_PLATFORM_WINDOWS)
    InitializeCriticalSection(&mutex->cs);
#endif

    TLTRACE("Mutex created 0x%p", mutex);
    TL_PROFILER_POP_WITH(mutex)
}

void tl_mutex_destroy(TLMutex* mutex) {
    TL_PROFILER_PUSH_WITH("0x%p", mutex)
    if (!mutex) {
        TLWARN("Attempted to destroy a NULL TLMutex")
        TL_PROFILER_POP
    }

#if defined(TL_PLATFORM_UNIX)
    i32 result = pthread_mutex_destroy(&mutex->mutex);
    if (result != 0) {
        TLERROR("tl_mutex_destroy: pthread_mutex_destroy failed with error %d", result);
    }
#elif defined(TL_PLATFORM_WINDOWS)
    DeleteCriticalSection(&mutex->cs);
#endif

    TLTRACE("Mutex destroyed 0x%p", mutex);
    tl_memory_free(mutex->allocator, mutex);
    TL_PROFILER_POP
}

b8 tl_mutex_lock(TLMutex* mutex) {
    TL_PROFILER_PUSH_WITH("0x%p", mutex)
    if (!mutex) {
        TLWARN("Attempted to lock a NULL TLMutex")
        TL_PROFILER_POP_WITH(false)
    }

#if defined(TL_PLATFORM_UNIX)
    i32 result = pthread_mutex_lock(&mutex->mutex);
    if (result != 0) {
        TLERROR("tl_mutex_lock: pthread_mutex_lock failed with error %d", result);
        return false;
    }
#elif defined(TL_PLATFORM_WINDOWS)
    EnterCriticalSection(&mutex->cs);
#endif
    TL_PROFILER_POP_WITH(true)
}

b8 tl_mutex_trylock(TLMutex* mutex) {
    TL_PROFILER_PUSH_WITH("0x%p", mutex)
    if (!mutex) {
        TLWARN("Attempted to try_lock a NULL TLMutex")
        TL_PROFILER_POP_WITH(false)
    }

    b8 locked = true;
#if defined(TL_PLATFORM_UNIX)
    i32 result = pthread_mutex_trylock(&mutex->mutex);
    if (result == EBUSY) {
        TL_PROFILER_POP_WITH(locked)  // Mutex already locked, not an error
    }

    if (result != 0) {
        TLERROR("tl_mutex_trylock: pthread_mutex_trylock failed with error %d", result);
        locked = false;
    }
#elif defined(TL_PLATFORM_WINDOWS)
    locked = TryEnterCriticalSection(&mutex->cs) != 0;
#endif
    TL_PROFILER_POP_WITH(locked)
}

b8 tl_mutex_unlock(TLMutex* mutex) {
    TL_PROFILER_PUSH_WITH("0x%p", mutex)
    if (!mutex) {
        TLWARN("Attempted to unlock a NULL TLMutex")
        TL_PROFILER_POP_WITH(false)
    }

#if defined(TL_PLATFORM_UNIX)
    i32 result = pthread_mutex_unlock(&mutex->mutex);
    if (result != 0) {
        TLERROR("tl_mutex_unlock: pthread_mutex_unlock failed with error %d", result);
        return false;
    }
#elif defined(TL_PLATFORM_WINDOWS)
    LeaveCriticalSection(&mutex->cs);
#endif
    TL_PROFILER_POP_WITH(true)
}

#endif