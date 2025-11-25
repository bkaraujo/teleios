#ifndef __TELEIOS_THREAD_TYPES__
#define __TELEIOS_THREAD_TYPES__

#   if defined(TL_PLATFORM_WINDOWS)
// ---------------------------------
// Internal structures
// ---------------------------------

#include <windows.h>
#include <process.h>

struct TLThread {
    HANDLE handle;
    u64 thread_id;
    TLThreadFunc func;
    void* arg;
    void* result;
    TLAllocator* allocator;
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
// Thread wrapper for pthread calling convention
// ---------------------------------

static DWORD WINAPI thread_wrapper(const LPVOID param) {
    TL_PROFILER_PUSH_WITH("0x%p", param)
    TLThread* thread = (TLThread*)param;
    thread->result = thread->func(thread->arg);
    TL_PROFILER_POP_WITH(0)
}
#   elif defined(TL_PLATFORM_UNIX)
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

// ---------------------------------
// Internal structures
// ---------------------------------

struct TLThread {
    pthread_t handle;
    TLThreadFunc func;
    void* arg;
    void* result;
    b8 detached;
    TLAllocator* allocator;
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
#   endif

#endif