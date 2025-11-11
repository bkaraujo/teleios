#ifdef TL_PLATFORM_WINDOWS

#include <windows.h>
#include <process.h>

// ---------------------------------
// Memory allocator for thread resources
// ---------------------------------

static TLAllocator* m_thread_allocator = NULL;

static void tl_thread_ensure_allocator(void) {
    if (m_thread_allocator != NULL) return;
    m_thread_allocator = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
    TLINFO("Thread system DYNAMIC allocator initialized");
}

// ---------------------------------
// Internal structures
// ---------------------------------

struct TLThread {
    HANDLE handle;
    u32 thread_id;
    TLThreadFunc func;
    void* arg;
    void* result;
};

struct TLMutex {
    TLAllocator* allocator;
    CRITICAL_SECTION cs;
};

struct TLCondition {
    TLAllocator* allocator;
    CONDITION_VARIABLE cv;
};

// ---------------------------------
// Thread wrapper for Windows calling convention
// ---------------------------------

static DWORD WINAPI thread_wrapper(const LPVOID param) {
    TLThread* thread = (TLThread*)param;
    thread->result = thread->func(thread->arg);
    return 0;
}

// ---------------------------------
// Thread Management
// ---------------------------------

TLThread* tl_thread_create(const TLThreadFunc func, void* arg) {
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

    DWORD tid;
    thread->handle = CreateThread(
        NULL,                   // Default security attributes
        0,                      // Default stack size
        thread_wrapper,         // Thread function
        thread,                 // Thread parameter
        0,                      // Start immediately
        &tid                    // Thread ID
    );

    if (!thread->handle) {
        TLERROR("tl_thread_create: CreateThread failed with error %lu", GetLastError());
        tl_memory_free(m_thread_allocator, thread);
        return NULL;
    }

    thread->thread_id = (u32)tid;
    TLTRACE("Thread created: ID=%u, Handle=%p", thread->thread_id, thread->handle);
    return thread;
}

b8 tl_thread_join(TLThread* thread, void** result) {
    if (!thread) {
        TLERROR("tl_thread_join: thread cannot be NULL");
        return false;
    }

    u32 wait_result = (u32)WaitForSingleObject(thread->handle, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        TLERROR("tl_thread_join: WaitForSingleObject failed with code %u", wait_result);
        return false;
    }

    if (result) {
        *result = thread->result;
    }

    CloseHandle(thread->handle);
    TLTRACE("Thread joined: ID=%u", thread->thread_id);
    tl_memory_free(m_thread_allocator, thread);
    return true;
}

b8 tl_thread_detach(TLThread* thread) {
    if (!thread) {
        TLERROR("tl_thread_detach: thread cannot be NULL");
        return false;
    }

    TLTRACE("Thread detached: ID=%u", thread->thread_id);
    CloseHandle(thread->handle);
    tl_memory_free(m_thread_allocator, thread);
    return true;
}

u64 tl_thread_id(void) {
    return (u64)GetCurrentThreadId();
}

void tl_thread_sleep(const u32 milliseconds) {
    Sleep(milliseconds);
}

// ---------------------------------
// Mutex (Mutual Exclusion)
// ---------------------------------

TLMutex* tl_mutex_create(TLAllocator* allocator) {
    TLMutex* mutex = (TLMutex*)tl_memory_alloc(allocator, TL_MEMORY_THREAD, sizeof(TLMutex));
    mutex->allocator = allocator;
    InitializeCriticalSection(&mutex->cs);
    TLTRACE("Mutex created:0x%p", mutex);
    return mutex;
}

void tl_mutex_destroy(TLMutex* mutex) {
    if (!mutex) {
        TLERROR("tl_mutex_destroy: mutex cannot be NULL");
        return;
    }

    DeleteCriticalSection(&mutex->cs);
    TLTRACE("Mutex destroyed:0x%p", mutex);
    tl_memory_free(mutex->allocator, mutex);
}

b8 tl_mutex_lock(TLMutex* mutex) {
    if (!mutex) {
        TLERROR("tl_mutex_lock: mutex cannot be NULL");
        return false;
    }

    EnterCriticalSection(&mutex->cs);
    return true;
}

b8 tl_mutex_trylock(TLMutex* mutex) {
    if (!mutex) {
        TLERROR("tl_mutex_trylock: mutex cannot be NULL");
        return false;
    }

    return TryEnterCriticalSection(&mutex->cs) != 0;
}

b8 tl_mutex_unlock(TLMutex* mutex) {
    if (!mutex) {
        TLERROR("tl_mutex_unlock: mutex cannot be NULL");
        return false;
    }

    LeaveCriticalSection(&mutex->cs);
    return true;
}

// ---------------------------------
// Condition Variables
// ---------------------------------

TLCondition* tl_condition_create(TLAllocator* allocator) {
    TLCondition* condition = (TLCondition*)tl_memory_alloc(allocator, TL_MEMORY_THREAD, sizeof(TLCondition));
    condition->allocator = allocator;
    InitializeConditionVariable(&condition->cv);
    TLTRACE("Condition variable created:0x%p", condition);
    return condition;
}

void tl_condition_destroy(TLCondition* condition) {
    if (!condition) {
        TLERROR("tl_condition_destroy: condition cannot be NULL");
        return;
    }

    // Windows condition variables don't need explicit cleanup
    TLTRACE("Condition variable destroyed:0x%p", condition);
    tl_memory_free(condition->allocator, condition);
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

    BOOL result = SleepConditionVariableCS(&condition->cv, &mutex->cs, INFINITE);
    if (!result) {
        TLERROR("tl_condition_wait: SleepConditionVariableCS failed with error %lu", GetLastError());
        return false;
    }

    return true;
}

b8 tl_condition_wait_timeout(TLCondition* condition, TLMutex* mutex, const u32 timeout_ms) {
    if (!condition) {
        TLERROR("tl_condition_wait_timeout: condition cannot be NULL");
        return false;
    }
    if (!mutex) {
        TLERROR("tl_condition_wait_timeout: mutex cannot be NULL");
        return false;
    }

    b8 result = (b8)SleepConditionVariableCS(&condition->cv, &mutex->cs, timeout_ms);
    if (!result) {
        u32 error = (u32)GetLastError();
        if (error == ERROR_TIMEOUT) {
            return false;  // Timeout, not an error
        }
        TLERROR("tl_condition_wait_timeout: SleepConditionVariableCS failed with error %u", error);
        return false;
    }

    return true;
}

b8 tl_condition_signal(TLCondition* condition) {
    if (!condition) {
        TLERROR("tl_condition_signal: condition cannot be NULL");
        return false;
    }

    WakeConditionVariable(&condition->cv);
    return true;
}

b8 tl_condition_broadcast(TLCondition* condition) {
    if (!condition) {
        TLERROR("tl_condition_broadcast: condition cannot be NULL");
        return false;
    }

    WakeAllConditionVariable(&condition->cv);
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

#endif // TL_PLATFORM_WINDOWS
