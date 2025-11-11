/**
 * @file container.h
 * @brief Thread-safe queue container data structure
 *
 * This module provides a circular queue (ring buffer) implementation for
 * storing and managing dynamic collections of void pointers.
 *
 * @section queue_characteristics Queue Characteristics
 *
 * The queue implementation provides:
 * - **Circular buffer design**: Efficient memory reuse
 * - **Fixed capacity**: Determined at creation time
 * - **Generic storage**: Holds void pointers to arbitrary data
 * - **Size tracking**: Queries for current size and capacity
 * - **State queries**: Check empty/full status
 * - **Clear operation**: Quick reset to empty state
 *
 * @section operations Queue Operations
 *
 * **Insertion:**
 * - `tl_queue_offer(queue, payload)` - Enqueue (push back)
 * - `tl_queue_push(queue, payload)` - Alternative enqueue name
 *
 * **Removal:**
 * - `tl_queue_pop(queue)` - Dequeue (pop front), returns data
 *
 * **Inspection:**
 * - `tl_queue_peek(queue)` - View front element without removal
 * - `tl_queue_size(queue)` - Get current number of elements
 * - `tl_queue_capacity(queue)` - Get maximum capacity
 * - `tl_queue_is_empty(queue)` - Check if empty
 * - `tl_queue_is_full(queue)` - Check if at capacity
 *
 * **Lifecycle:**
 * - `tl_queue_create(allocator, capacity)` - Create new queue
 * - `tl_queue_destroy(queue)` - Delete and cleanup
 * - `tl_queue_clear(queue)` - Empty queue (keep capacity)
 *
 * @section memory_management Memory Management
 *
 * Queues use allocators for memory management:
 * - Created with custom allocator (LINEAR or DYNAMIC)
 * - Memory tagged as TL_MEMORY_CONTAINER_QUEUE
 * - Allocator must remain valid for queue lifetime
 * - Destroying queue deallocates internal structures
 *
 * @section usage Usage Examples
 *
 * **Creating and using a queue:**
 * @code
 * #include "teleios/container.h"
 * #include "teleios/memory.h"
 *
 * // Create allocator
 * TLAllocator* heap = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
 *
 * // Create queue with capacity for 100 items
 * TLQueue* queue = tl_queue_create(heap, 100);
 *
 * if (queue == NULL) {
 *     TLERROR("Failed to create queue");
 *     return false;
 * }
 *
 * // Check initial state
 * TLINFO("Queue created: capacity=%d, size=%d",
 *     tl_queue_capacity(queue),
 *     tl_queue_size(queue));
 *
 * // Cleanup
 * tl_queue_destroy(queue);
 * tl_memory_allocator_destroy(heap);
 * @endcode
 *
 * **Enqueueing and dequeuing:**
 * @code
 * // Create queue
 * TLAllocator* heap = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
 * TLQueue* queue = tl_queue_create(heap, 50);
 *
 * // Enqueue some integers (store as void pointers)
 * for (int i = 0; i < 10; ++i) {
 *     tl_queue_offer(queue, (void*)(intptr_t)i);
 * }
 *
 * // Check queue status
 * if (tl_queue_is_full(queue)) {
 *     TLINFO("Queue is full");
 * } else {
 *     TLINFO("Queue has space: %d / %d items",
 *         tl_queue_size(queue),
 *         tl_queue_capacity(queue));
 * }
 *
 * // Peek at front
 * void* front = tl_queue_peek(queue);
 * TLINFO("Front element: %d", (int)(intptr_t)front);
 *
 * // Dequeue all items
 * while (!tl_queue_is_empty(queue)) {
 *     void* item = tl_queue_pop(queue);
 *     int value = (int)(intptr_t)item;
 *     TLINFO("Dequeued: %d", value);
 * }
 * @endcode
 *
 * **Working with struct pointers:**
 * @code
 * struct Message {
 *     u32 type;
 *     char text[256];
 * };
 *
 * // Create queue
 * TLAllocator* heap = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
 * TLQueue* message_queue = tl_queue_create(heap, 100);
 *
 * // Enqueue messages
 * struct Message* msg1 = malloc(sizeof(struct Message));
 * msg1->type = 1;
 * snprintf(msg1->text, sizeof(msg1->text), "Hello");
 * tl_queue_offer(message_queue, msg1);
 *
 * struct Message* msg2 = malloc(sizeof(struct Message));
 * msg2->type = 2;
 * snprintf(msg2->text, sizeof(msg2->text), "World");
 * tl_queue_offer(message_queue, msg2);
 *
 * // Process messages
 * while (!tl_queue_is_empty(message_queue)) {
 *     struct Message* msg = (struct Message*)tl_queue_pop(message_queue);
 *     TLINFO("Message type %u: %s", msg->type, msg->text);
 *     free(msg);
 * }
 *
 * // Cleanup
 * tl_queue_destroy(message_queue);
 * tl_memory_allocator_destroy(heap);
 * @endcode
 *
 * **Clearing and reusing:**
 * @code
 * TLQueue* queue = tl_queue_create(heap, 100);
 *
 * // Add some items
 * for (int i = 0; i < 50; ++i) {
 *     tl_queue_offer(queue, (void*)(intptr_t)i);
 * }
 *
 * TLINFO("Queue size before clear: %d", tl_queue_size(queue));
 *
 * // Clear the queue
 * tl_queue_clear(queue);
 *
 * TLINFO("Queue size after clear: %d", tl_queue_size(queue));
 * TLINFO("Queue capacity after clear: %d (unchanged)", tl_queue_capacity(queue));
 *
 * // Can reuse immediately
 * tl_queue_offer(queue, (void*)(intptr_t)123);
 * @endcode
 *
 * @note The queue holds void pointers - you must manage the data they point to.
 *       The queue does NOT own or manage the pointed-to memory.
 *
 * @note For small integral values (< 2^31), cast directly: (void*)(intptr_t)value
 *
 * @note Queue capacity is fixed at creation and cannot be resized dynamically.
 *       Choose appropriate capacity when creating.
 *
 * @note There is no built-in synchronization. For multi-threaded use, add
 *       external locking with mutexes or use thread-local queues.
 *
 * @see memory.h - Memory allocators (LINEAR, DYNAMIC)
 *
 * @author TELEIOS Team
 * @version 0.1.0
 */

#ifndef __TELEIOS_CONTAINER__
#define __TELEIOS_CONTAINER__

#include "teleios/defines.h"

// =================================
// QUEUE API
// =================================

/**
 * @brief Opaque queue data structure handle
 *
 * Represents a circular queue instance. The actual structure definition
 * is in the implementation file (container.c).
 */
typedef struct TLQueue TLQueue;

/**
 * @brief Create a new queue with specified capacity
 *
 * Allocates and initializes a circular queue that can hold up to the
 * specified number of void pointers.
 *
 * @param allocator Memory allocator to use (must be valid and remain alive)
 * @param capacity Maximum number of items the queue can hold
 * @return Pointer to new queue, or NULL on allocation failure
 *
 * @note The allocator must remain valid for the queue's entire lifetime
 * @note Capacity is fixed and cannot be changed after creation
 * @note Queue memory is tagged as TL_MEMORY_CONTAINER_QUEUE
 * @note Initial queue state is empty
 *
 * @see tl_queue_destroy
 *
 * @code
 * TLAllocator* heap = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
 * TLQueue* queue = tl_queue_create(heap, 256);
 *
 * if (queue == NULL) {
 *     TLERROR("Queue creation failed");
 *     return false;
 * }
 * @endcode
 */
TLQueue* tl_queue_create(TLAllocator* allocator, u16 capacity);

/**
 * @brief Destroy a queue and free its memory
 *
 * Deallocates the queue structure. The queue should be empty before
 * destruction, or stored pointers will be lost.
 *
 * @param queue Queue to destroy (may be NULL)
 *
 * @note Safe to call with NULL (no-op)
 * @note Data pointed to by queue items is NOT freed (queue only holds pointers)
 * @note After destruction, using the queue is undefined behavior
 *
 * @see tl_queue_create
 *
 * @code
 * TLQueue* queue = tl_queue_create(heap, 100);
 *
 * // ... use queue ...
 *
 * // Empty it first if needed
 * while (!tl_queue_is_empty(queue)) {
 *     void* item = tl_queue_pop(queue);
 *     free(item);  // Free pointed-to data if needed
 * }
 *
 * tl_queue_destroy(queue);
 * @endcode
 */
void tl_queue_destroy(TLQueue* queue);

/**
 * @brief Enqueue (add to back) an item to the queue
 *
 * Adds a pointer to the back of the queue. If the queue is full, behavior
 * is undefined (will likely overwrite the oldest item).
 *
 * @param queue Queue to add to
 * @param payload Void pointer to enqueue (can be NULL)
 *
 * @note Does not check if queue is full - caller must verify with tl_queue_is_full()
 * @note Payload is stored as-is; the queue doesn't take ownership
 * @note Intptral values can be stored: tl_queue_offer(q, (void*)(intptr_t)value)
 *
 * @see tl_queue_push - Alias for tl_queue_offer
 * @see tl_queue_is_full - Check capacity before offering
 * @see tl_queue_pop - Remove from front
 *
 * @code
 * if (!tl_queue_is_full(queue)) {
 *     tl_queue_offer(queue, some_pointer);
 * } else {
 *     TLWARN("Queue is full, cannot enqueue");
 * }
 * @endcode
 */
void tl_queue_offer(TLQueue* queue, void* payload);

/**
 * @brief Enqueue (add to back) an item to the queue (alias for tl_queue_offer)
 *
 * Alias for tl_queue_offer() with different naming. Both are equivalent.
 *
 * @param queue Queue to add to
 * @param payload Void pointer to enqueue
 *
 * @see tl_queue_offer - Primary function
 * @see tl_queue_pop - Remove from front
 */
void tl_queue_push(TLQueue* queue, void* payload);

/**
 * @brief Dequeue (remove from front) an item from the queue
 *
 * Removes and returns the item at the front of the queue. The queue must
 * not be empty.
 *
 * @param queue Queue to remove from
 * @return The void pointer that was at the front of the queue
 *
 * @note Behavior is undefined if queue is empty - caller must check with tl_queue_is_empty()
 * @note The returned pointer is the original value that was enqueued
 * @note Queue does not manage the lifetime of pointed-to data
 *
 * @see tl_queue_is_empty - Check if queue has items
 * @see tl_queue_peek - View front without removing
 * @see tl_queue_offer - Add to back
 *
 * @code
 * if (!tl_queue_is_empty(queue)) {
 *     void* item = tl_queue_pop(queue);
 *     process_item(item);
 * }
 * @endcode
 */
void* tl_queue_pop(TLQueue* queue);

/**
 * @brief Peek at (view) the front item without removing it
 *
 * Returns the item at the front of the queue without removing it.
 * The queue must not be empty.
 *
 * @param queue Queue to peek into
 * @return The void pointer at the front of the queue
 *
 * @note Behavior is undefined if queue is empty
 * @note Does not modify the queue
 * @note Returned pointer remains in queue until tl_queue_pop() is called
 *
 * @see tl_queue_is_empty - Check if queue has items
 * @see tl_queue_pop - Remove and get front item
 *
 * @code
 * if (!tl_queue_is_empty(queue)) {
 *     void* front = tl_queue_peek(queue);
 *     if (should_process(front)) {
 *         tl_queue_pop(queue);  // Now remove it
 *     }
 * }
 * @endcode
 */
void* tl_queue_peek(TLQueue* queue);

/**
 * @brief Get current number of items in queue
 *
 * Returns how many items are currently in the queue.
 *
 * @param queue Queue to query
 * @return Number of items currently in queue (0 to capacity)
 *
 * @see tl_queue_capacity - Get maximum capacity
 * @see tl_queue_is_empty - Check if size is 0
 * @see tl_queue_is_full - Check if size equals capacity
 *
 * @code
 * TLINFO("Queue: %u / %u items",
 *     tl_queue_size(queue),
 *     tl_queue_capacity(queue));
 * @endcode
 */
u16 tl_queue_size(const TLQueue* queue);

/**
 * @brief Get maximum capacity of queue
 *
 * Returns the capacity that was specified when the queue was created.
 * The queue cannot hold more than this many items.
 *
 * @param queue Queue to query
 * @return Maximum capacity in items
 *
 * @note Capacity is fixed and cannot be changed
 * @note Current size may be less than capacity (use tl_queue_size() to check)
 *
 * @see tl_queue_size - Get current item count
 * @see tl_queue_is_full - Check if size equals capacity
 *
 * @code
 * if (tl_queue_size(queue) == tl_queue_capacity(queue)) {
 *     TLWARN("Queue is full");
 * }
 * @endcode
 */
u16 tl_queue_capacity(const TLQueue* queue);

/**
 * @brief Check if queue is empty
 *
 * Returns true if the queue contains no items.
 *
 * @param queue Queue to check
 * @return true if queue is empty, false if it contains items
 *
 * @see tl_queue_size - Get actual item count
 * @see tl_queue_is_full - Check if queue is at capacity
 *
 * @code
 * while (!tl_queue_is_empty(queue)) {
 *     void* item = tl_queue_pop(queue);
 *     process(item);
 * }
 * @endcode
 */
b8 tl_queue_is_empty(const TLQueue* queue);

/**
 * @brief Check if queue is full
 *
 * Returns true if the queue contains the maximum number of items.
 *
 * @param queue Queue to check
 * @return true if queue is at capacity, false otherwise
 *
 * @note A full queue will reject new items (though offer doesn't check this)
 *
 * @see tl_queue_is_empty - Check if queue is empty
 * @see tl_queue_size - Get current item count
 * @see tl_queue_capacity - Get maximum capacity
 *
 * @code
 * if (!tl_queue_is_full(queue)) {
 *     tl_queue_offer(queue, new_item);
 * }
 * @endcode
 */
b8 tl_queue_is_full(const TLQueue* queue);

/**
 * @brief Clear the queue (remove all items without destroying it)
 *
 * Empties the queue, removing all items. The queue capacity remains unchanged
 * and the queue can be immediately reused.
 *
 * @param queue Queue to clear
 *
 * @note Capacity is not changed - queue remains usable after clear
 * @note Data pointed to by removed items is NOT freed
 * @note This is faster than destroying and recreating the queue
 *
 * @see tl_queue_destroy - Deallocate the queue entirely
 * @see tl_queue_is_empty - Check if already empty
 *
 * @code
 * // Process all items
 * while (!tl_queue_is_empty(queue)) {
 *     void* item = tl_queue_pop(queue);
 *     process_and_free(item);
 * }
 *
 * // Or clear at once if not processing
 * tl_queue_clear(queue);
 * @endcode
 */
void tl_queue_clear(TLQueue* queue);

// =================================
// OBJECT POOL API
// =================================

/**
 * @brief Opaque object pool handle
 *
 * Represents a pool of pre-allocated objects for efficient reuse.
 * The actual structure definition is in the implementation file (container.c).
 */
typedef struct TLObjectPool TLObjectPool;

/**
 * @brief Create a new object pool with pre-allocated objects
 *
 * Allocates and initializes a pool of objects that can be acquired and released
 * for efficient reuse. All objects are pre-allocated during pool creation.
 *
 * @param allocator Memory allocator to use (must be valid and remain alive)
 * @param object_size Size in bytes of each object in the pool
 * @param capacity Number of objects to pre-allocate
 * @return Pointer to new object pool, or NULL on allocation failure
 *
 * @note All objects in the pool are zero-initialized
 * @note Pool capacity is fixed and cannot be changed after creation
 * @note Pool memory is tagged as TL_MEMORY_CONTAINER_POOL
 * @note Thread-safe - uses internal mutex for synchronization
 *
 * @see tl_pool_destroy
 * @see tl_pool_acquire
 *
 * @code
 * TLAllocator* heap = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
 * TLObjectPool* pool = tl_pool_create(heap, sizeof(MyObject), 256);
 *
 * if (pool == NULL) {
 *     TLERROR("Pool creation failed");
 *     return false;
 * }
 * @endcode
 */
TLObjectPool* tl_pool_create(TLAllocator* allocator, u32 object_size, u16 capacity);

/**
 * @brief Destroy an object pool and free all memory
 *
 * Deallocates the pool and all pre-allocated objects. Pool should ideally
 * have all objects released before destruction.
 *
 * @param pool Pool to destroy (may be NULL)
 *
 * @note Safe to call with NULL (no-op)
 * @note Does not call destructors - objects are simply freed
 * @note After destruction, using the pool is undefined behavior
 *
 * @see tl_pool_create
 *
 * @code
 * TLObjectPool* pool = tl_pool_create(heap, sizeof(Task), 100);
 * // ... use pool ...
 * tl_pool_destroy(pool);
 * @endcode
 */
void tl_pool_destroy(TLObjectPool* pool);

/**
 * @brief Acquire an object from the pool
 *
 * Returns a pointer to an available object from the pool. The object is
 * marked as in-use and will not be returned by subsequent acquires until
 * it is released back to the pool.
 *
 * @param pool Pool to acquire from
 * @return Pointer to acquired object, or NULL if pool is exhausted
 *
 * @note Returns NULL if no objects are available (pool exhausted)
 * @note Returned object retains its previous data - caller should initialize
 * @note Thread-safe - can be called concurrently from multiple threads
 * @note Acquired objects must be released with tl_pool_release()
 *
 * @see tl_pool_release
 * @see tl_pool_available
 *
 * @code
 * MyObject* obj = (MyObject*)tl_pool_acquire(pool);
 * if (obj == NULL) {
 *     TLWARN("Pool exhausted, no objects available");
 *     return;
 * }
 *
 * // Initialize and use object
 * obj->value = 42;
 * process(obj);
 *
 * // Release back to pool when done
 * tl_pool_release(pool, obj);
 * @endcode
 */
void* tl_pool_acquire(TLObjectPool* pool);

/**
 * @brief Release an object back to the pool
 *
 * Marks an object as available for reuse. The object must have been
 * previously acquired from this pool.
 *
 * @param pool Pool to release to
 * @param object Object to release (must be from this pool)
 *
 * @note Object data is NOT cleared - next acquire will see previous data
 * @note Releasing an object that wasn't acquired is undefined behavior
 * @note Thread-safe - can be called concurrently from multiple threads
 * @note Object pointer remains valid but should not be used after release
 *
 * @see tl_pool_acquire
 *
 * @code
 * MyObject* obj = (MyObject*)tl_pool_acquire(pool);
 * // ... use object ...
 * tl_pool_release(pool, obj);  // Return to pool for reuse
 * @endcode
 */
void tl_pool_release(TLObjectPool* pool, void* object);

/**
 * @brief Get number of available (free) objects in pool
 *
 * Returns how many objects can still be acquired from the pool.
 *
 * @param pool Pool to query
 * @return Number of available objects (0 to capacity)
 *
 * @note Available count may change if pool is accessed concurrently
 * @note Returns 0 if pool is exhausted
 *
 * @see tl_pool_capacity
 * @see tl_pool_in_use
 *
 * @code
 * u16 available = tl_pool_available(pool);
 * if (available == 0) {
 *     TLWARN("Pool exhausted");
 * }
 * @endcode
 */
u16 tl_pool_available(const TLObjectPool* pool);

/**
 * @brief Get number of objects currently in use
 *
 * Returns how many objects have been acquired and not yet released.
 *
 * @param pool Pool to query
 * @return Number of objects in use (0 to capacity)
 *
 * @see tl_pool_available
 * @see tl_pool_capacity
 *
 * @code
 * TLINFO("Pool usage: %u / %u objects in use",
 *     tl_pool_in_use(pool),
 *     tl_pool_capacity(pool));
 * @endcode
 */
u16 tl_pool_in_use(const TLObjectPool* pool);

/**
 * @brief Get total capacity of pool
 *
 * Returns the total number of objects that were pre-allocated during
 * pool creation.
 *
 * @param pool Pool to query
 * @return Total pool capacity
 *
 * @note Capacity is fixed and never changes
 * @note capacity = available + in_use
 *
 * @see tl_pool_available
 * @see tl_pool_in_use
 *
 * @code
 * u16 capacity = tl_pool_capacity(pool);
 * TLINFO("Pool created with capacity: %u", capacity);
 * @endcode
 */
u16 tl_pool_capacity(const TLObjectPool* pool);

/**
 * @brief Reset pool to initial state (release all objects)
 *
 * Marks all objects as available, regardless of their current state.
 * Does not reinitialize object data.
 *
 * @param pool Pool to reset
 *
 * @note Does not clear object data - objects retain their state
 * @note All acquired objects become available again
 * @note Use with caution - pointers to acquired objects become invalid
 *
 * @see tl_pool_create
 *
 * @code
 * // Reset pool to reuse all objects
 * tl_pool_reset(pool);
 * TLINFO("Pool reset: %u objects available", tl_pool_available(pool));
 * @endcode
 */
void tl_pool_reset(TLObjectPool* pool);

#endif
