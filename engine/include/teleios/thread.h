#ifndef __TELEIOS_THREAD__
#define __TELEIOS_THREAD__

#include "teleios/defines.h"

/**
 * Thread function signature
 * @param arg User-provided argument passed to the thread
 * @return Thread result (can be retrieved via tl_thread_join)
 */
typedef void* (*TLThreadFunction)(void* arg);

// ---------------------------------
// Thread Management
// ---------------------------------

/**
 * Creates and starts a new thread
 * @param allocator Allocator
 * @param func Function to execute in the thread
 * @param arg Argument to pass to the thread function
 * @return Pointer to TLThread on success, NULL on failure
 */
TLThread* tl_thread_create(TLAllocator* allocator, TLThreadFunction func, void* arg);

/**
 * Waits for a thread to finish execution
 * @param thread Thread to wait for
 * @param result Optional pointer to store the thread's return value (can be NULL)
 * @return b8 true on success, false on failure
 */
b8 tl_thread_join(TLThread* thread, void** result);

/**
 * Detaches a thread (thread resources are freed automatically when it terminates)
 * @param thread Thread to detach
 * @return b8 true on success, false on failure
 */
b8 tl_thread_detach(TLThread* thread);

/**
 * Gets the thread's ID
 * @return Thread ID as u64
 */
u64 tl_thread_id(TLThread* thread);

/**
 * Gets the current thread's ID
 * @return Thread ID as u64
 */
u64 tl_thread_current_id(void);

/**
 * Sleeps the current thread for specified milliseconds
 * @param milliseconds Number of milliseconds to sleep
 */
void tl_thread_sleep(u32 milliseconds);

// ---------------------------------
// Mutex (Mutual Exclusion)
// ---------------------------------

/**
 * Creates a new mutex
 * @return Pointer to TLMutex on success, NULL on failure
 */
TLMutex* tl_mutex_create(TLAllocator* allocator);

/**
 * Destroys a mutex and frees its resources
 * @param mutex Mutex to destroy
 */
void tl_mutex_destroy(TLMutex* mutex);

/**
 * Locks a mutex (blocks until lock is acquired)
 * @param mutex Mutex to lock
 * @return b8 true on success, false on failure
 */
b8 tl_mutex_lock(TLMutex* mutex);

/**
 * Attempts to lock a mutex without blocking
 * @param mutex Mutex to try locking
 * @return b8 true if lock acquired, false if already locked or error
 */
b8 tl_mutex_trylock(TLMutex* mutex);

/**
 * Unlocks a mutex
 * @param mutex Mutex to unlock
 * @return b8 true on success, false on failure
 */
b8 tl_mutex_unlock(TLMutex* mutex);

// ---------------------------------
// Condition Variables
// ---------------------------------

/**
 * Creates a new condition variable
 * @return Pointer to TLCondition on success, NULL on failure
 */
TLCondition* tl_condition_create(TLAllocator* allocator);

/**
 * Destroys a condition variable and frees its resources
 * @param condition Condition variable to destroy
 */
void tl_condition_destroy(TLCondition* condition);

/**
 * Waits on a condition variable (atomically unlocks mutex and waits)
 * @param condition Condition variable to wait on
 * @param mutex Mutex associated with the condition (must be locked)
 * @return b8 true on success, false on failure
 */
b8 tl_condition_wait(TLCondition* condition, TLMutex* mutex);

/**
 * Waits on a condition variable with timeout
 * @param condition Condition variable to wait on
 * @param mutex Mutex associated with the condition (must be locked)
 * @param timeout_ms Timeout in milliseconds
 * @return b8 true if signaled, false if timeout or error
 */
b8 tl_condition_wait_timeout(TLCondition* condition, TLMutex* mutex, u32 timeout_ms);

/**
 * Signals one waiting thread on the condition variable
 * @param condition Condition variable to signal
 * @return b8 true on success, false on failure
 */
b8 tl_condition_signal(TLCondition* condition);

/**
 * Signals all waiting threads on the condition variable
 * @param condition Condition variable to broadcast
 * @return b8 true on success, false on failure
 */
b8 tl_condition_broadcast(TLCondition* condition);

#endif
