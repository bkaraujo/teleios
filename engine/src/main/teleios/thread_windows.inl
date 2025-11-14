#ifdef TL_PLATFORM_WINDOWS

#include <windows.h>
#include <process.h>

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
    TL_PROFILER_PUSH_WITH("0x%p", param)
    TLThread* thread = (TLThread*)param;
    thread->result = thread->func(thread->arg);
    TL_PROFILER_POP_WITH(0)
}

// ---------------------------------
// Thread Management
// ---------------------------------

TLThread* tl_thread_create(const TLThreadFunc func, void* arg) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", func, arg)
    if (!func) {
        TLERROR("tl_thread_create: func cannot be NULL");
        TL_PROFILER_POP_WITH(NULL)
    }

    TLThread* thread = (TLThread*)tl_memory_alloc(g_allocator, TL_MEMORY_THREAD, sizeof(TLThread));

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
        tl_memory_free(g_allocator, thread);
        TL_PROFILER_POP_WITH(NULL)
    }

    thread->thread_id = (u32)tid;
    TLTRACE("Thread created: ID=%u, Handle=%p", thread->thread_id, thread->handle);
    TL_PROFILER_POP_WITH(thread)
}

b8 tl_thread_join(TLThread* thread, void** result) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", thread, result)
    if (!thread) {
        TLERROR("tl_thread_join: thread cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }

    const u32 wait_result = (u32)WaitForSingleObject(thread->handle, INFINITE);
    if (wait_result != WAIT_OBJECT_0) {
        TLERROR("tl_thread_join: WaitForSingleObject failed with code %u", wait_result);
        TL_PROFILER_POP_WITH(false)
    }

    if (result) {
        *result = thread->result;
    }

    CloseHandle(thread->handle);
    TLTRACE("Thread joined: ID=%u", thread->thread_id);
    tl_memory_free(g_allocator, thread);
    TL_PROFILER_POP_WITH(true)
}

b8 tl_thread_detach(TLThread* thread) {
    TL_PROFILER_PUSH_WITH("0x%p", thread)
    if (!thread) {
        TLERROR("tl_thread_detach: thread cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }

    TLTRACE("Thread detached: ID=%u", thread->thread_id);
    CloseHandle(thread->handle);
    tl_memory_free(g_allocator, thread);
    TL_PROFILER_POP_WITH(true)
}

u64 tl_thread_id(void) {
    return (u64)GetCurrentThreadId();
}

void tl_thread_sleep(const u32 milliseconds) {
    TL_PROFILER_PUSH
    Sleep(milliseconds);
    TL_PROFILER_POP
}

// ---------------------------------
// Mutex (Mutual Exclusion)
// ---------------------------------

TLMutex* tl_mutex_create(TLAllocator* allocator) {
    TL_PROFILER_PUSH_WITH("0x%p", allocator)
    if (allocator == NULL) {
        TLWARN("Attempt to use a NULL TLAllocator")
        TL_PROFILER_POP_WITH(NULL)
    }

    TLMutex* mutex = (TLMutex*)tl_memory_alloc(allocator, TL_MEMORY_THREAD, sizeof(TLMutex));
    mutex->allocator = allocator;
    InitializeCriticalSection(&mutex->cs);
    TLTRACE("Mutex created:0x%p", mutex);

    TL_PROFILER_POP_WITH(mutex)
}

void tl_mutex_destroy(TLMutex* mutex) {
    TL_PROFILER_PUSH_WITH("0x%p", mutex)
    if (!mutex) {
        TLERROR("tl_mutex_destroy: mutex cannot be NULL");
        TL_PROFILER_POP
    }

    DeleteCriticalSection(&mutex->cs);
    TLTRACE("Mutex destroyed:0x%p", mutex);
    tl_memory_free(mutex->allocator, mutex);
    TL_PROFILER_POP
}

b8 tl_mutex_lock(TLMutex* mutex) {
    TL_PROFILER_PUSH_WITH("0x%p", mutex)
    if (!mutex) {
        TLERROR("tl_mutex_lock: mutex cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }

    EnterCriticalSection(&mutex->cs);
    TL_PROFILER_POP_WITH(true)
}

b8 tl_mutex_trylock(TLMutex* mutex) {
    TL_PROFILER_PUSH_WITH("0x%p", mutex)
    if (!mutex) {
        TLERROR("tl_mutex_trylock: mutex cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }

    TL_PROFILER_POP_WITH(TryEnterCriticalSection(&mutex->cs) != 0)
}

b8 tl_mutex_unlock(TLMutex* mutex) {
    TL_PROFILER_PUSH_WITH("0x%p", mutex)
    if (!mutex) {
        TLERROR("tl_mutex_unlock: mutex cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }

    LeaveCriticalSection(&mutex->cs);
    TL_PROFILER_POP_WITH(true)
}

// ---------------------------------
// Condition Variables
// ---------------------------------

TLCondition* tl_condition_create(TLAllocator* allocator) {
    TL_PROFILER_PUSH_WITH("0x%p", allocator)
    if (allocator == NULL) {
        TLWARN("Attempt to use a NULL TLAllocator")
        TL_PROFILER_POP_WITH(NULL)
    }

    TLCondition* condition = (TLCondition*)tl_memory_alloc(allocator, TL_MEMORY_THREAD, sizeof(TLCondition));
    condition->allocator = allocator;
    InitializeConditionVariable(&condition->cv);
    TLTRACE("Condition variable created:0x%p", condition);
    TL_PROFILER_POP_WITH(condition);
}

void tl_condition_destroy(TLCondition* condition) {
    TL_PROFILER_PUSH_WITH("0x%p", condition)
    if (!condition) {
        TLERROR("tl_condition_destroy: condition cannot be NULL");
        TL_PROFILER_POP
    }

    // Windows condition variables don't need explicit cleanup
    TLTRACE("Condition variable destroyed:0x%p", condition);
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

    const BOOL result = SleepConditionVariableCS(&condition->cv, &mutex->cs, INFINITE);
    if (!result) {
        TLERROR("tl_condition_wait: SleepConditionVariableCS failed with error %lu", GetLastError());
        TL_PROFILER_POP_WITH(false)
    }

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

    const b8 result = (b8)SleepConditionVariableCS(&condition->cv, &mutex->cs, timeout_ms);
    if (!result) {
        const u32 error = (u32)GetLastError();
        if (error == ERROR_TIMEOUT) {
            TL_PROFILER_POP_WITH(false)
        }
        TLERROR("tl_condition_wait_timeout: SleepConditionVariableCS failed with error %u", error);
        TL_PROFILER_POP_WITH(false)
    }

    TL_PROFILER_POP_WITH(true)
}

b8 tl_condition_signal(TLCondition* condition) {
    TL_PROFILER_PUSH_WITH("0x%p", condition)
    if (!condition) {
        TLERROR("tl_condition_signal: condition cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }

    WakeConditionVariable(&condition->cv);
    TL_PROFILER_POP_WITH(true)
}

b8 tl_condition_broadcast(TLCondition* condition) {
    TL_PROFILER_PUSH_WITH("0x%p", condition)
    if (!condition) {
        TLERROR("tl_condition_broadcast: condition cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }

    WakeAllConditionVariable(&condition->cv);
    TL_PROFILER_POP_WITH(true)
}


#endif
