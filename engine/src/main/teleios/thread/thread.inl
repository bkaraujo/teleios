#ifndef __TELEIOS_THREAD_THREAD__
#define __TELEIOS_THREAD_THREAD__
#include "teleios/teleios.h"
#include "teleios/thread/types.inl"

// ---------------------------------
// Thread Management
// ---------------------------------

TLThread* tl_thread_create(TLAllocator* allocator, const TLThreadFunc func, void* arg) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", func, arg)
    if (!func) {
        TLERROR("tl_thread_create: func cannot be NULL");
        TL_PROFILER_POP_WITH(NULL)
    }

    TLThread* thread = (TLThread*)tl_memory_alloc(allocator, TL_MEMORY_THREAD, sizeof(TLThread));
    thread->allocator = allocator;
    thread->func = func;
    thread->arg = arg;

#if defined(TL_PLATFORM_UNIX)
    thread->detached = false;

    i32 result = pthread_create(&thread->handle, NULL, thread_wrapper, thread);
    if (result != 0) {
        TLERROR("tl_thread_create: pthread_create failed with error %d", result);
        tl_memory_free(thread->allocator, thread);
        TL_PROFILER_POP_WITH(NULL)
    }

    TLDEBUG("Thread created: ID=%llu", (u64)thread->handle);
#elif defined(TL_PLATFORM_WINDOWS)
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
        tl_memory_free(allocator, thread);
        TL_PROFILER_POP_WITH(NULL)
    }

    thread->thread_id = (u32)tid;
    TLTRACE("Thread created: ID=%u, Handle=%p", thread->thread_id, thread->handle);
#endif

    TL_PROFILER_POP_WITH(thread)
}

b8 tl_thread_join(TLThread* thread, void** result) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", thread, result)
    if (!thread) {
        TLERROR("tl_thread_join: thread cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }

#if defined(TL_PLATFORM_UNIX)
    if (thread->detached) {
        TLERROR("tl_thread_join: Cannot join a detached thread");
        TL_PROFILER_POP_WITH(false)
    }

    void* thread_result;
    i32 join_result = pthread_join(thread->handle, &thread_result);
    if (join_result != 0) {
        TLERROR("tl_thread_join: pthread_join failed with error %d", join_result);
        TL_PROFILER_POP_WITH(false)
    }

    if (result) {
        *result = thread->result;
    }

    TLDEBUG("Thread joined: ID=%llu", (u64)thread->handle);
#elif defined(TL_PLATFORM_WINDOWS)
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
#endif
    tl_memory_free(thread->allocator, thread);
    TL_PROFILER_POP_WITH(true)
}

b8 tl_thread_detach(TLThread* thread) {
    TL_PROFILER_PUSH_WITH("0x%p", thread)
    if (!thread) {
        TLERROR("tl_thread_detach: thread cannot be NULL");
        TL_PROFILER_POP_WITH(false)
    }

#if defined(TL_PLATFORM_UNIX)
    if (thread->detached) {
        TLERROR("tl_thread_detach: Thread already detached");
        TL_PROFILER_POP_WITH(false)
    }

    i32 result = pthread_detach(thread->handle);
    if (result != 0) {
        TLERROR("tl_thread_detach: pthread_detach failed with error %d", result);
        TL_PROFILER_POP_WITH(false)
    }

    thread->detached = true;
    TLDEBUG("Thread detached: ID=%llu", (u64)thread->handle);
#elif defined(TL_PLATFORM_WINDOWS)
    TLTRACE("Thread detached: ID=%u", thread->thread_id);
    CloseHandle(thread->handle);
#endif

    tl_memory_free(thread->allocator, thread);
    TL_PROFILER_POP_WITH(true)
}

u64 tl_thread_id(void) {
#if defined(TL_PLATFORM_UNIX)
    return (u64)pthread_self();
#elif defined(TL_PLATFORM_WINDOWS)
    return (u64)GetCurrentThreadId();
#endif
}

void tl_thread_sleep(const u32 milliseconds) {
    TL_PROFILER_PUSH
#if defined(TL_PLATFORM_UNIX)
    usleep(milliseconds * 1000);  // usleep takes microseconds
#elif defined(TL_PLATFORM_WINDOWS)
    Sleep(milliseconds);
#endif
    TL_PROFILER_POP
}

#endif
