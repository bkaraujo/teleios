/**
 * @file thread.c
 * @brief Cross-platform threading implementation
 *
 * ARCHITECTURE PATTERN: Compile-Time Platform Selection
 *
 * Unlike the memory system which uses runtime dispatch (switch/case on allocator type),
 * the thread system uses compile-time dispatch via #ifdef guards because:
 *
 * 1. Platform is known at compile time (no runtime overhead)
 * 2. Platform-specific types cannot be abstracted (HANDLE vs pthread_t)
 * 3. Dead code elimination removes unused platform code
 *
 * STRUCTURE:
 *   thread.h               - Public API with forward declarations
 *   thread.c               - This file (compile-time dispatcher)
 *   thread_windows.inl     - Windows implementation (HANDLE, CRITICAL_SECTION, CONDITION_VARIABLE)
 *   thread_unix.inl        - Unix/POSIX implementation (pthread_t, pthread_mutex_t, pthread_cond_t)
 *
 * MEMORY MANAGEMENT:
 *   Both implementations use DYNAMIC allocator (TL_ALLOCATOR_DYNAMIC) for:
 *   - Thread structures (TLThread)
 *   - Mutex structures (TLMutex)
 *   - Condition variable structures (TLCondition)
 *
 *   This allows individual deallocation and automatic leak detection on shutdown.
 *
 * PATTERN COMPARISON:
 *   Memory System:  Runtime dispatch (enum type → switch/case → implementation)
 *   Platform System: Compile-time dispatch (function pointers initialized via #ifdef)
 *   Thread System:   Compile-time dispatch (direct #ifdef → include .inl)
 *
 * WHY NO DISPATCHER FUNCTION TABLE?
 *   Thread APIs are called frequently (lock/unlock, wait/signal), so we want zero
 *   overhead. Direct compile-time selection eliminates any indirection.
 */

#include "teleios.h"
#include "thread.h"

// Include platform-specific implementations
#if defined(TL_PLATFORM_WINDOWS)
    #include "thread_windows.inl"
#elif defined(TL_PLATFORM_LINUX) || defined(TL_PLATFORM_APPLE) || defined(TL_PLATFORM_UNIX)
    #include "thread_unix.inl"
#else
    #error "Unsupported platform for threading"
#endif
