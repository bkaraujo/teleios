/**
 * @file thread_windows.inl
 * @brief Windows-specific threading implementation
 *
 * Uses Windows API: CreateThread, CRITICAL_SECTION, CONDITION_VARIABLE
 * This file is included directly into thread.c
 */

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
    CRITICAL_SECTION cs;
};

struct TLCondition {
    CONDITION_VARIABLE cv;
};

// ---------------------------------
// Thread wrapper for Windows calling convention
// ---------------------------------

static DWORD WINAPI thread_wrapper(LPVOID param) {
    TLThread* thread = (TLThread*)param;
    thread->result = thread->func(thread->arg);
    return 0;
}

// ---------------------------------
// Thread Management
// ---------------------------------

TLThread* tl_thread_create(TLThreadFunc func, void* arg) {
    if (!func) {
        TLERROR("tl_thread_create: func cannot be NULL");
        return NULL;
    }

    TLThread* thread = (TLThread*)malloc(sizeof(TLThread));
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
        free(thread);
        return NULL;
    }

    thread->thread_id = (u32)tid;
    TLDEBUG("Thread created: ID=%u, Handle=%p", thread->thread_id, thread->handle);
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
    TLDEBUG("Thread joined: ID=%u", thread->thread_id);
    free(thread);
    return true;
}

b8 tl_thread_detach(TLThread* thread) {
    if (!thread) {
        TLERROR("tl_thread_detach: thread cannot be NULL");
        return false;
    }

    TLDEBUG("Thread detached: ID=%u", thread->thread_id);
    CloseHandle(thread->handle);
    free(thread);
    return true;
}

u64 tl_thread_id(void) {
    return (u64)GetCurrentThreadId();
}

void tl_thread_sleep(u32 milliseconds) {
    Sleep(milliseconds);
}

// ---------------------------------
// Mutex (Mutual Exclusion)
// ---------------------------------

TLMutex* tl_mutex_create(void) {
    TLMutex* mutex = (TLMutex*)malloc(sizeof(TLMutex));
    if (!mutex) {
        TLERROR("tl_mutex_create: Failed to allocate mutex structure");
        return NULL;
    }

    InitializeCriticalSection(&mutex->cs);
    TLDEBUG("Mutex created: %p", mutex);
    return mutex;
}

void tl_mutex_destroy(TLMutex* mutex) {
    if (!mutex) {
        TLERROR("tl_mutex_destroy: mutex cannot be NULL");
        return;
    }

    DeleteCriticalSection(&mutex->cs);
    TLDEBUG("Mutex destroyed: %p", mutex);
    free(mutex);
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

TLCondition* tl_condition_create(void) {
    TLCondition* condition = (TLCondition*)malloc(sizeof(TLCondition));
    if (!condition) {
        TLERROR("tl_condition_create: Failed to allocate condition structure");
        return NULL;
    }

    InitializeConditionVariable(&condition->cv);
    TLDEBUG("Condition variable created: %p", condition);
    return condition;
}

void tl_condition_destroy(TLCondition* condition) {
    if (!condition) {
        TLERROR("tl_condition_destroy: condition cannot be NULL");
        return;
    }

    // Windows condition variables don't need explicit cleanup
    TLDEBUG("Condition variable destroyed: %p", condition);
    free(condition);
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

b8 tl_condition_wait_timeout(TLCondition* condition, TLMutex* mutex, u32 timeout_ms) {
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

#endif // TL_PLATFORM_WINDOWS
