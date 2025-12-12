#ifndef __TELEIOS_CONTAINER__
#define __TELEIOS_CONTAINER__

#include "teleios/defines.h"

// =================================
// DYNAMIC ARRAY API
// =================================

/**
 * @brief Create a new dynamic array of void pointers
 *
 * Allocates and initializes a dynamic array that stores void pointers.
 * The array automatically grows when capacity is exceeded.
 * The array does NOT take ownership of the pointed-to data.
 *
 * @param allocator Memory allocator to use (must be valid and remain alive)
 * @param initial_capacity Initial number of pointers to allocate space for
 * @param thread_safe Whether to use mutex for thread-safe operations
 * @return Pointer to new array, or NULL on allocation failure
 *
 * @note The allocator must remain valid for the array's entire lifetime
 * @note Array memory is tagged as TL_MEMORY_CONTAINER_ARRAY
 * @note If capacity is 0, defaults to 8
 * @note Array stores pointers only - caller manages pointed-to data lifetime
 *
 * @see tl_array_destroy
 *
 * @code
 * TLAllocator* heap = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
 * TLArray* items = tl_array_create(heap, 16, true);
 *
 * MyStruct* obj = create_object();
 * tl_array_push(items, obj);
 * @endcode
 */
TLArray* tl_array_create(TLAllocator* allocator, u32 initial_capacity, b8 thread_safe);

/**
 * @brief Destroy an array and free its memory
 *
 * Deallocates the array structure and internal storage.
 *
 * @param array Array to destroy (may be NULL)
 *
 * @note Safe to call with NULL (no-op)
 * @note Does not call destructors on elements - only frees memory
 * @note After destruction, using the array is undefined behavior
 *
 * @see tl_array_create
 */
void tl_array_destroy(TLArray* array);

/**
 * @brief Add a pointer to the end of the array
 *
 * Stores the pointer at the end of the array. Automatically grows capacity if needed.
 *
 * @param array Array to add to
 * @param item Pointer to store (array does NOT take ownership)
 * @return true on success, false on allocation failure
 *
 * @note Thread-safe if created with thread_safe=true
 * @note Pointer is stored as-is (not copied)
 * @note Amortized O(1) time complexity
 * @note May trigger reallocation
 *
 * @see tl_array_pop
 * @see tl_array_insert
 *
 * @code
 * MyStruct* obj = create_object();
 * if (!tl_array_push(array, obj)) {
 *     TLERROR("Failed to push element");
 * }
 * @endcode
 */
b8 tl_array_push(TLArray* array, void* item);

/**
 * @brief Remove and return the last pointer from the array
 *
 * Removes and returns the last pointer from the array.
 *
 * @param array Array to remove from
 * @return The removed pointer, or NULL if array is empty
 *
 * @note Thread-safe if created with thread_safe=true
 * @note O(1) time complexity
 * @note Caller is responsible for freeing the pointed-to data if needed
 *
 * @see tl_array_push
 * @see tl_array_remove
 *
 * @code
 * MyStruct* obj = tl_array_pop(array);
 * if (obj != NULL) {
 *     process(obj);
 *     free(obj);
 * }
 * @endcode
 */
void* tl_array_pop(TLArray* array);

/**
 * @brief Get the pointer at the specified index
 *
 * Returns the pointer stored at the given index.
 *
 * @param array Array to query
 * @param index Index of pointer (0-based)
 * @return The stored pointer, or NULL if index is out of bounds
 *
 * @note Thread-safe if created with thread_safe=true
 * @note O(1) time complexity
 *
 * @see tl_array_set
 *
 * @code
 * MyStruct* obj = tl_array_get(array, 5);
 * if (obj != NULL) {
 *     process(obj);
 * }
 * @endcode
 */
void* tl_array_get(TLArray* array, u32 index);

/**
 * @brief Set the pointer at the specified index
 *
 * Replaces the pointer at the specified index.
 *
 * @param array Array to modify
 * @param index Index to set (0-based, must be < count)
 * @param item Pointer to store (array does NOT take ownership)
 * @return true on success, false if index is out of bounds
 *
 * @note Thread-safe if created with thread_safe=true
 * @note O(1) time complexity
 * @note Index must be less than current count
 * @note Does NOT free the previous pointer - caller must manage memory
 *
 * @see tl_array_get
 * @see tl_array_insert
 *
 * @code
 * MyStruct* new_obj = create_object();
 * if (!tl_array_set(array, 5, new_obj)) {
 *     TLERROR("Index out of bounds");
 * }
 * @endcode
 */
b8 tl_array_set(TLArray* array, u32 index, void* item);

/**
 * @brief Insert a pointer at the specified index
 *
 * Inserts the pointer at the given index, shifting subsequent pointers forward.
 *
 * @param array Array to modify
 * @param index Index to insert at (0-based, may equal count for append)
 * @param item Pointer to store (array does NOT take ownership)
 * @return true on success, false on allocation failure or invalid index
 *
 * @note Thread-safe if created with thread_safe=true
 * @note O(n) time complexity (due to shifting)
 * @note Index may equal count (equivalent to push)
 * @note May trigger reallocation
 *
 * @see tl_array_remove
 * @see tl_array_push
 *
 * @code
 * MyStruct* obj = create_object();
 * if (!tl_array_insert(array, 3, obj)) {
 *     TLERROR("Failed to insert element");
 * }
 * @endcode
 */
b8 tl_array_insert(TLArray* array, u32 index, void* item);

/**
 * @brief Remove the element matching the given pointer
 *
 * Searches for the given pointer in the array and removes it, shifting subsequent pointers backward.
 *
 * @param array Array to modify
 * @param element Pointer to element to remove (must be in array)
 *
 * @note Thread-safe if created with thread_safe=true
 * @note O(n) time complexity (due to search and shifting)
 * @note Caller is responsible for freeing the pointed-to data if needed
 * @note Removes first occurrence if multiple identical pointers exist
 *
 * @see tl_array_insert
 * @see tl_array_pop
 *
 * @code
 * MyStruct* obj = create_object();
 * tl_array_push(array, obj);
 * b8 removed = tl_array_remove(array, obj);
 * if (!removed){
 *    TLWARN("Item not in erray")
 * }
 * @endcode
 */
b8 tl_array_remove(TLArray* array, void* element);

/**
 * @brief Get current number of elements in array
 *
 * Returns how many elements are currently stored in the array.
 *
 * @param array Array to query
 * @return Number of elements (0 if array is NULL)
 *
 * @note Thread-safe - uses internal mutex
 *
 * @see tl_array_capacity
 * @see tl_array_is_empty
 */
u32 tl_array_size(const TLArray* array);

/**
 * @brief Get current capacity of array
 *
 * Returns the number of elements that can be stored before reallocation is needed.
 *
 * @param array Array to query
 * @return Current capacity (0 if array is NULL)
 *
 * @note Thread-safe - uses internal mutex
 * @note Capacity >= size always
 *
 * @see tl_array_size
 * @see tl_array_reserve
 */
u32 tl_array_capacity(const TLArray* array);

/**
 * @brief Check if array is empty
 *
 * Returns true if the array contains no elements.
 *
 * @param array Array to check
 * @return true if array is empty or NULL, false otherwise
 *
 * @see tl_array_size
 */
b8 tl_array_is_empty(const TLArray* array);

/**
 * @brief Remove all elements from array
 *
 * Clears all elements but retains allocated capacity.
 *
 * @param array Array to clear
 *
 * @note Thread-safe - uses internal mutex
 * @note Does not free memory - use tl_array_shrink_to_fit() after if needed
 * @note O(1) time complexity
 *
 * @see tl_array_shrink_to_fit
 */
void tl_array_clear(TLArray* array);

/**
 * @brief Create an iterator for the array
 *
 * Creates a fail-fast iterator that traverses the array from index 0 to size-1.
 * The iterator creates a snapshot of the array's modification count at creation time.
 *
 * @param array Array to iterate over
 * @return Pointer to new iterator, or NULL on allocation failure
 *
 * @note Thread-safe - captures array state atomically
 * @note Iterator becomes invalid if array is modified after creation
 * @note Concurrent modification will trigger FATAL error
 * @note Must call tl_iterator_destroy() when done
 * @note Use tl_iterator_resync() to update iterator after modifications
 *
 * @see tl_iterator_has_next
 * @see tl_iterator_next
 * @see tl_iterator_rewind
 * @see tl_iterator_resync
 * @see tl_iterator_destroy
 *
 * @code
 * TLArray* array = tl_array_create(allocator, sizeof(i32), 10);
 * // ... add elements ...
 *
 * TLIterator* iter = tl_array_iterator(array);
 * while (tl_iterator_has_next(iter)) {
 *     i32* value = (i32*)tl_iterator_next(iter);
 *     TLINFO("Value: %d", *value);
 * }
 * tl_iterator_destroy(iter);
 * @endcode
 */
TLIterator* tl_array_iterator(TLArray* array);

// =================================
// QUEUE API
// =================================

/**
 * @brief Create a new queue with specified capacity
 *
 * Allocates and initializes a circular queue that can hold up to the
 * specified number of void pointers.
 *
 * @param allocator Memory allocator to use (must be valid and remain alive)
 * @param capacity Maximum number of items the queue can hold
 * @param thread_safe Whether to use mutex for thread-safe operations
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
 * TLQueue* queue = tl_queue_create(heap, 256, true);
 *
 * if (queue == NULL) {
 *     TLERROR("Queue creation failed");
 *     return false;
 * }
 * @endcode
 */
TLQueue* tl_queue_create(TLAllocator* allocator, u16 capacity, b8 thread_safe);

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
 * TLQueue* queue = tl_queue_create(heap, 100, true);
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
 * Removes and returns the item at the front of the queue, or else return NULL
 *
 * @param queue Queue to remove from
 * @return The void pointer that was at the front of the queue or NULL
 *
 * @note If enmpty returns NULL
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

/**
 * @brief Create an iterator for the queue with fail-fast behavior
 *
 * Creates an iterator that tracks the current state of the queue at creation time.
 * If the queue is modified during iteration, the iterator will detect this and
 * trigger a FATAL error.
 *
 * @param queue Queue to iterate over
 * @return Pointer to new iterator, or NULL on allocation failure
 *
 * @note Thread-safe - uses queue mutex during creation
 * @note Iterator must be destroyed with tl_iterator_destroy()
 * @note Modifying queue during iteration causes FATAL error (fail-fast)
 * @note Iteration follows FIFO order (tail to head)
 *
 * @see tl_iterator_destroy
 * @see tl_iterator_next
 * @see tl_iterator_has_next
 *
 * @code
 * TLIterator* iter = tl_queue_iterator(queue);
 *
 * while (tl_iterator_has_next(iter)) {
 *     void* item = tl_iterator_next(iter);
 *     process_item(item);
 *     // DO NOT modify queue here - will cause FATAL error
 * }
 *
 * tl_iterator_destroy(iter);
 * @endcode
 */
TLIterator* tl_queue_iterator(TLQueue* queue);

// =================================
// OBJECT POOL API
// =================================

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
TLObjectPool* tl_pool_create(TLAllocator* allocator, u32 object_size, u16 capacity, b8 thread_safe);

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
 * @see tl_pool_acquire_wait
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
 * @brief Acquire an object from the pool, blocking if empty
 *
 * Returns a pointer to an available object from the pool. If no objects
 * are available, blocks until one is released back to the pool.
 *
 * @param pool Pool to acquire from (must be thread-safe)
 * @return Pointer to acquired object (never NULL)
 *
 * @note Blocks indefinitely if pool is exhausted until an object is released
 * @note Pool MUST be created with thread_safe=true
 * @note Returned object retains its previous data - caller should initialize
 * @note Thread-safe - uses internal condition variable for synchronization
 * @note Acquired objects must be released with tl_pool_release()
 *
 * @see tl_pool_acquire
 * @see tl_pool_release
 * @see tl_pool_available
 *
 * @code
 * // Producer-consumer pattern
 * void* consumer_thread(void* arg) {
 *     TLObjectPool* pool = (TLObjectPool*)arg;
 *
 *     while (running) {
 *         // Blocks until an object is available
 *         MyObject* obj = (MyObject*)tl_pool_acquire_wait(pool);
 *
 *         // Process the object
 *         process(obj);
 *
 *         // Release back to pool
 *         tl_pool_release(pool, obj);
 *     }
 *     return NULL;
 * }
 * @endcode
 */
void* tl_pool_acquire_wait(TLObjectPool* pool);

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

/**
 * @brief Create an iterator for the pool with fail-fast behavior
 *
 * Creates an iterator that tracks the current state of the pool at creation time.
 * If the pool is modified during iteration (acquire/release), the iterator will
 * detect this and trigger a FATAL error.
 *
 * @param pool Pool to iterate over
 * @return Pointer to new iterator, or NULL on allocation failure
 *
 * @note Thread-safe - uses pool mutex during creation
 * @note Iterator must be destroyed with tl_iterator_destroy()
 * @note Modifying pool during iteration causes FATAL error (fail-fast)
 * @note Only iterates over currently acquired objects (in-use objects)
 * @note Iteration order is index-sequential (0 to capacity-1)
 *
 * @see tl_iterator_destroy
 * @see tl_iterator_next
 * @see tl_iterator_has_next
 *
 * @code
 * TLIterator* iter = tl_pool_iterator(pool);
 *
 * while (tl_iterator_has_next(iter)) {
 *     void* object = tl_iterator_next(iter);
 *     MyObject* obj = (MyObject*)object;
 *     process_object(obj);
 *     // DO NOT acquire/release from pool here - will cause FATAL error
 * }
 *
 * tl_iterator_destroy(iter);
 * @endcode
 */
TLIterator* tl_pool_iterator(TLObjectPool* pool);

// =================================
// DOUBLE LINKED LIST API
// =================================

/**
 * @brief Create a new empty double linked list
 *
 * Allocates and initializes an empty double linked list with bidirectional
 * traversal support. Nodes can be inserted and removed from both ends efficiently.
 *
 * @param allocator Memory allocator to use (must be valid and remain alive)
 * @return Pointer to new list, or NULL on allocation failure
 *
 * @note The allocator must remain valid for the list's entire lifetime
 * @note List memory is tagged as TL_MEMORY_CONTAINER_LIST
 * @note Thread-safe - uses internal mutex for synchronization
 * @note Initial list state is empty (head and tail are NULL)
 *
 * @see tl_list_destroy
 *
 * @code
 * TLAllocator* heap = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
 * TLList* list = tl_list_create(heap);
 *
 * if (list == NULL) {
 *     TLERROR("List creation failed");
 *     return false;
 * }
 * @endcode
 */
TLList* tl_list_create(TLAllocator* allocator, b8 thread_safe);

/**
 * @brief Destroy a list and free all nodes
 *
 * Deallocates the list structure and all nodes. Data pointed to by nodes
 * is not freed (list only holds pointers).
 *
 * @param list List to destroy (may be NULL)
 *
 * @note Safe to call with NULL (no-op)
 * @note Data pointed to by nodes is NOT freed (list only holds pointers)
 * @note After destruction, using the list is undefined behavior
 *
 * @see tl_list_create
 *
 * @code
 * TLList* list = tl_list_create(heap);
 * // ... use list ...
 *
 * // Free data if needed before destroying list
 * TLListNode* node = tl_list_front(list);
 * while (node != NULL) {
 *     void* data = tl_list_data(node);
 *     free(data);  // Free pointed-to data if needed
 *     node = tl_list_next(node);
 * }
 *
 * tl_list_destroy(list);
 * @endcode
 */
void tl_list_destroy(TLList* list);

/**
 * @brief Create an iterator for the list with fail-fast behavior
 *
 * Creates an iterator that tracks the current state of the list at creation time.
 * If the list is modified during iteration, the iterator will detect this and
 * trigger a FATAL error.
 *
 * @param list List to iterate over
 * @return Pointer to new iterator, or NULL on allocation failure
 *
 * @note Thread-safe - uses list mutex during creation
 * @note Iterator must be destroyed with tl_iterator_destroy()
 * @note Modifying list during iteration causes FATAL error (fail-fast)
 * @note Iteration follows forward direction (head to tail)
 *
 * @see tl_iterator_destroy
 * @see tl_iterator_next
 * @see tl_iterator_has_next
 *
 * @code
 * TLIterator* iter = tl_list_iterator(list);
 *
 * if (iter == NULL) {
 *     TLERROR("Iterator creation failed");
 *     return;
 * }
 *
 * while (tl_iterator_has_next(iter)) {
 *     void* item = tl_iterator_next(iter);
 *     process_item(item);
 *     // DO NOT modify list here - will cause FATAL error
 * }
 *
 * tl_iterator_destroy(iter);
 * @endcode
 */
TLIterator* tl_list_iterator(TLList* list);

/**
 * @brief Insert data at the front of the list
 *
 * Creates a new node with the given data and inserts it at the beginning
 * of the list. Operation is O(1).
 *
 * @param list List to insert into
 * @param data Void pointer to store (can be NULL)
 * @return Pointer to newly created node, or NULL on allocation failure
 *
 * @note Thread-safe - uses internal mutex
 * @note Data is stored as-is; the list doesn't take ownership
 * @note Returns newly created node for potential later reference
 *
 * @see tl_list_push_back
 * @see tl_list_pop_front
 *
 * @code
 * TLListNode* node = tl_list_push_front(list, some_pointer);
 * if (node == NULL) {
 *     TLERROR("Failed to insert node");
 * }
 * @endcode
 */
void tl_list_push_front(TLList* list, void* data);

/**
 * @brief Insert data at the back of the list
 *
 * Creates a new node with the given data and inserts it at the end
 * of the list. Operation is O(1).
 *
 * @param list List to insert into
 * @param data Void pointer to store (can be NULL)
 * @return Pointer to newly created node, or NULL on allocation failure
 *
 * @note Thread-safe - uses internal mutex
 * @note Data is stored as-is; the list doesn't take ownership
 * @note Returns newly created node for potential later reference
 *
 * @see tl_list_push_front
 * @see tl_list_pop_back
 *
 * @code
 * TLListNode* node = tl_list_push_back(list, some_pointer);
 * if (node == NULL) {
 *     TLERROR("Failed to insert node");
 * }
 * @endcode
 */
void tl_list_push_back(TLList* list, void* data);

/**
 * @brief Insert data after a specific node
 *
 * Creates a new node with the given data and inserts it immediately
 * after the specified node. Operation is O(1).
 *
 * @param list List containing the node
 * @param node Node to insert after (must be in this list)
 * @param data Void pointer to store (can be NULL)
 * @return Pointer to newly created node, or NULL on allocation failure
 *
 * @note Thread-safe - uses internal mutex
 * @note If node is NULL, behaves like tl_list_push_back
 * @note Inserting after tail makes the new node the new tail
 *
 * @see tl_list_insert_before
 * @see tl_list_push_back
 *
 * @code
 * TLListNode* middle = tl_list_front(list);
 * TLListNode* new_node = tl_list_insert_after(list, middle, new_data);
 * @endcode
 */
void tl_list_insert_after(TLList* list, TLListNode* node, void* data);

/**
 * @brief Insert data before a specific node
 *
 * Creates a new node with the given data and inserts it immediately
 * before the specified node. Operation is O(1).
 *
 * @param list List containing the node
 * @param node Node to insert before (must be in this list)
 * @param data Void pointer to store (can be NULL)
 * @return Pointer to newly created node, or NULL on allocation failure
 *
 * @note Thread-safe - uses internal mutex
 * @note If node is NULL, behaves like tl_list_push_front
 * @note Inserting before head makes the new node the new head
 *
 * @see tl_list_insert_after
 * @see tl_list_push_front
 *
 * @code
 * TLListNode* middle = tl_list_back(list);
 * TLListNode* new_node = tl_list_insert_before(list, middle, new_data);
 * @endcode
 */
void tl_list_insert_before(TLList* list, TLListNode* node, void* data);

/**
 * @brief Remove and return data from the front of the list
 *
 * Removes the first node and returns its data. The list must not be empty.
 * Operation is O(1).
 *
 * @param list List to remove from
 * @return The void pointer stored in the removed node
 *
 * @note Behavior is undefined if list is empty - caller must check with tl_list_is_empty()
 * @note Thread-safe - uses internal mutex
 * @note The node is freed, but the data is not
 *
 * @see tl_list_is_empty
 * @see tl_list_push_front
 * @see tl_list_front
 *
 * @code
 * if (!tl_list_is_empty(list)) {
 *     void* data = tl_list_pop_front(list);
 *     process_data(data);
 * }
 * @endcode
 */
void* tl_list_pop_front(TLList* list);

/**
 * @brief Remove and return data from the back of the list
 *
 * Removes the last node and returns its data. The list must not be empty.
 * Operation is O(1).
 *
 * @param list List to remove from
 * @return The void pointer stored in the removed node
 *
 * @note Behavior is undefined if list is empty - caller must check with tl_list_is_empty()
 * @note Thread-safe - uses internal mutex
 * @note The node is freed, but the data is not
 *
 * @see tl_list_is_empty
 * @see tl_list_push_back
 * @see tl_list_back
 *
 * @code
 * if (!tl_list_is_empty(list)) {
 *     void* data = tl_list_pop_back(list);
 *     process_data(data);
 * }
 * @endcode
 */
void* tl_list_pop_back(TLList* list);

/**
 * @brief Remove a specific node from the list
 *
 * Removes the given node from the list and returns its data.
 * Operation is O(1).
 *
 * @param list List containing the node
 * @param node Node to remove (must be in this list)
 * @return The void pointer stored in the removed node
 *
 * @note Thread-safe - uses internal mutex
 * @note The node is freed, but the data is not
 * @note Removing the only node makes the list empty
 *
 * @see tl_list_pop_front
 * @see tl_list_pop_back
 *
 * @code
 * TLListNode* node = tl_list_front(list);
 * while (node != NULL) {
 *     TLListNode* next = tl_list_next(node);
 *     if (should_remove(tl_list_data(node))) {
 *         void* data = tl_list_remove(list, node);
 *         free(data);
 *     }
 *     node = next;
 * }
 * @endcode
 */
void* tl_list_remove(TLList* list, TLListNode* node);

/**
 * @brief Get the first node in the list
 *
 * Returns the head node without modifying the list.
 *
 * @param list List to query
 * @return Pointer to first node, or NULL if list is empty
 *
 * @note Does not remove the node
 * @note Thread-safe - uses internal mutex
 *
 * @see tl_list_back
 * @see tl_list_next
 * @see tl_list_pop_front
 *
 * @code
 * TLListNode* node = tl_list_front(list);
 * if (node != NULL) {
 *     void* data = tl_list_data(node);
 *     process_data(data);
 * }
 * @endcode
 */
void* tl_list_front(TLList* list);

/**
 * @brief Get the last node in the list
 *
 * Returns the tail node without modifying the list.
 *
 * @param list List to query
 * @return Pointer to last node, or NULL if list is empty
 *
 * @note Does not remove the node
 * @note Thread-safe - uses internal mutex
 *
 * @see tl_list_front
 * @see tl_list_prev
 * @see tl_list_pop_back
 *
 * @code
 * TLListNode* node = tl_list_back(list);
 * if (node != NULL) {
 *     void* data = tl_list_data(node);
 *     process_data(data);
 * }
 * @endcode
 */
void* tl_list_back(TLList* list);

/**
 * @brief Get current number of nodes in list
 *
 * Returns how many nodes are currently in the list.
 *
 * @param list List to query
 * @return Number of nodes currently in list
 *
 * @note Thread-safe - uses internal mutex
 *
 * @see tl_list_is_empty
 *
 * @code
 * TLINFO("List contains %u nodes", tl_list_size(list));
 * @endcode
 */
u32 tl_list_size(TLList* list);

/**
 * @brief Check if list is empty
 *
 * Returns true if the list contains no nodes.
 *
 * @param list List to check
 * @return true if list is empty, false if it contains nodes
 *
 * @note Thread-safe - uses internal mutex
 *
 * @see tl_list_size
 *
 * @code
 * if (tl_list_is_empty(list)) {
 *     TLINFO("List is empty");
 * } else {
 *     process_list(list);
 * }
 * @endcode
 */
b8 tl_list_is_empty(TLList* list);

/**
 * @brief Clear the list (remove all nodes without destroying it)
 *
 * Removes and frees all nodes in the list. The list structure remains
 * valid and can be immediately reused.
 *
 * @param list List to clear
 *
 * @note Data pointed to by removed nodes is NOT freed
 * @note Thread-safe - uses internal mutex
 * @note This is faster than destroying and recreating the list
 *
 * @see tl_list_destroy
 * @see tl_list_is_empty
 *
 * @code
 * // Free all data before clearing
 * TLListNode* node = tl_list_front(list);
 * while (node != NULL) {
 *     void* data = tl_list_data(node);
 *     free(data);
 *     node = tl_list_next(node);
 * }
 *
 * // Clear the list
 * tl_list_clear(list);
 * @endcode
 */
void tl_list_clear(TLList* list);

// =================================
// HASHMAP API (TLString -> void*)
// =================================

/**
 * @brief Create a new hash map with TLString keys and TLList* values
 *
 * Allocates and initializes a hash map with separate chaining for collision
 * resolution. Each key is a TLString and each value is a TLList of void*.
 *
 * @param allocator Memory allocator to use (must be valid and remain alive)
 * @param capacity Initial number of buckets (will be rounded up to power of 2)
 * @return Pointer to new map, or NULL on allocation failure
 *
 * @note The allocator must remain valid for the map's entire lifetime
 * @note Map memory is tagged as TL_MEMORY_CONTAINER_MAP
 * @note Thread-safe - uses internal mutex for synchronization
 * @note Default load factor is 0.75 (map resizes when 75% full)
 * @note Keys and values are owned by the map and will be freed on destroy
 *
 * @see tl_map_destroy
 *
 * @code
 * TLAllocator* heap = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
 * TLMap* map = tl_map_create(heap, 16);
 *
 * if (map == NULL) {
 *     TLERROR("Map creation failed");
 *     return false;
 * }
 * @endcode
 */
TLMap* tl_map_create(TLAllocator* allocator, u32 capacity, b8 thread_safe);

/**
 * @brief Destroy a map and free all keys and values
 *
 * Deallocates the map structure, all keys (TLString), and all values (TLList).
 * Data pointed to by list items is NOT freed.
 *
 * @param map Map to destroy (may be NULL)
 *
 * @note Safe to call with NULL (no-op)
 * @note All TLString keys are destroyed
 * @note All TLList values are destroyed (but not the data they contain)
 * @note After destruction, using the map is undefined behavior
 *
 * @see tl_map_create
 *
 * @code
 * TLMap* map = tl_map_create(heap, 16);
 * // ... use map ...
 * tl_map_destroy(map);
 * @endcode
 */
void tl_map_destroy(TLMap* map);

/**
 * @brief Create an iterator for the map keys with fail-fast behavior
 *
 * Creates an iterator that tracks the current state of the map at creation time.
 * If the map is modified during iteration, the iterator will detect this and
 * trigger a FATAL error.
 *
 * @param map Map to iterate over
 * @return Pointer to new iterator, or NULL on allocation failure
 *
 * @note Thread-safe - uses map mutex during creation
 * @note Iterator must be destroyed with tl_iterator_destroy()
 * @note Modifying map during iteration causes FATAL error (fail-fast)
 * @note Returns TLString* keys during iteration
 * @note Iteration order is bucket-sequential (unordered)
 *
 * @see tl_iterator_destroy
 * @see tl_iterator_next
 * @see tl_iterator_has_next
 *
 * @code
 * TLIterator* iter = tl_map_keys(map);
 *
 * if (iter == NULL) {
 *     TLERROR("Iterator creation failed");
 *     return;
 * }
 *
 * while (tl_iterator_has_next(iter)) {
 *     TLString* key = (TLString*)tl_iterator_next(iter);
 *     TLList* values = tl_map_get(map, key);
 *     process_values(values);
 *     // DO NOT modify map here - will cause FATAL error
 * }
 *
 * tl_iterator_destroy(iter);
 * @endcode
 */
TLIterator* tl_map_keys(TLMap* map);

/**
 * @brief Get the list associated with a key
 *
 * Returns the TLList* associated with the given key. If the key doesn't exist,
 * returns NULL.
 *
 * @param map Map to query
 * @param key Key to look up
 * @return TLList* associated with key, or NULL if key not found
 *
 * @note Thread-safe - uses internal mutex
 * @note Returned list pointer is valid until the key is removed or map is destroyed
 * @note The returned list can be modified directly (it's not a copy)
 *
 * @see tl_map_get_or_create
 * @see tl_map_contains
 *
 * @code
 * TLString* key = tl_string_create(heap, "users");
 * TLList* users = tl_map_get(map, key);
 *
 * if (users != NULL) {
 *     // List exists for this key
 *     tl_list_push_back(users, user_data);
 * }
 *
 * tl_string_destroy(key);
 * @endcode
 */
TLList* tl_map_get(TLMap* map, const TLString* key);

/**
 * @brief Get the list for a key, creating it if it doesn't exist
 *
 * Returns the TLList* associated with the given key. If the key doesn't exist,
 * creates a new entry with an empty list.
 *
 * @param map Map to modify
 * @param key Key to look up or create
 * @return TLList* associated with key (never NULL unless map is NULL)
 *
 * @note Thread-safe - uses internal mutex
 * @note If key doesn't exist, a copy of the key is stored in the map
 * @note The returned list is owned by the map
 * @note May trigger a resize if load factor is exceeded
 *
 * @see tl_map_get
 * @see tl_map_remove
 *
 * @code
 * TLString* key = tl_string_create(heap, "users");
 * TLList* users = tl_map_get_or_create(map, key);
 *
 * // Always safe to use the list (it's created if needed)
 * tl_list_push_back(users, user_data);
 *
 * tl_string_destroy(key);
 * @endcode
 */
TLList* tl_map_get_or_create(TLMap* map, const TLString* key);

/**
 * @brief Put a value into the map using a C string key
 *
 * Convenience function that creates a TLString key from a C string,
 * gets or creates the list for that key, and adds the value to it.
 *
 * @param map Map to modify
 * @param key String key
 * @param value Value to add (will be added to the list for this key)
 *
 * @note Thread-safe - uses internal mutex
 * @note Creates a new key TLString if key doesn't exist
 * @note Value is added to the list associated with the key
 * @note Key TLString is created internally (temporary allocation)
 *
 * @see tl_map_get_or_create
 * @see tl_list_push_back
 *
 * @code
 * TLString* value = tl_string_create(heap, "John");
 * tl_map_put(map, "user.name", value);
 * @endcode
 */
void tl_map_put(TLMap* map, TLString* key, void* value);

/**
 * @brief Check if map contains a key
 *
 * Returns true if the map contains an entry for the given key.
 *
 * @param map Map to query
 * @param key Key to check
 * @return true if key exists, false otherwise
 *
 * @note Thread-safe - uses internal mutex
 * @note Faster than tl_map_get() when you only need to check existence
 *
 * @see tl_map_get
 *
 * @code
 * TLString* key = tl_string_create(heap, "users");
 *
 * if (tl_map_contains(map, key)) {
 *     TLINFO("Users list exists");
 * } else {
 *     TLINFO("No users list yet");
 * }
 *
 * tl_string_destroy(key);
 * @endcode
 */
b8 tl_map_contains(TLMap* map, const TLString* key);

/**
 * @brief Remove a key-value pair from the map
 *
 * Removes the entry for the given key and returns its list. The list is
 * detached from the map but not destroyed.
 *
 * @param map Map to modify
 * @param key Key to remove
 * @return TLList* that was associated with the key, or NULL if key not found
 *
 * @note Thread-safe - uses internal mutex
 * @note The TLString key is destroyed
 * @note The returned TLList* must be destroyed by the caller with tl_list_destroy()
 * @note Returns NULL if key doesn't exist
 *
 * @see tl_map_clear
 * @see tl_map_destroy
 *
 * @code
 * TLString* key = tl_string_create(heap, "users");
 * TLList* users = tl_map_remove(map, key);
 *
 * if (users != NULL) {
 *     // Clean up list contents
 *     while (!tl_list_is_empty(users)) {
 *         void* data = tl_list_pop_front(users);
 *         free(data);
 *     }
 *     tl_list_destroy(users);
 * }
 *
 * tl_string_destroy(key);
 * @endcode
 */
TLList* tl_map_remove(TLMap* map, const TLString* key);

/**
 * @brief Get current number of key-value pairs in map
 *
 * Returns how many entries are currently in the map.
 *
 * @param map Map to query
 * @return Number of key-value pairs
 *
 * @note Thread-safe - uses internal mutex
 *
 * @see tl_map_capacity
 * @see tl_map_is_empty
 *
 * @code
 * TLINFO("Map contains %u entries", tl_map_size(map));
 * @endcode
 */
u32 tl_map_size(TLMap* map);

/**
 * @brief Get current capacity (number of buckets) of map
 *
 * Returns the current number of buckets in the map's internal hash table.
 *
 * @param map Map to query
 * @return Number of buckets
 *
 * @note Thread-safe - uses internal mutex
 * @note Capacity may increase automatically when load factor is exceeded
 *
 * @see tl_map_size
 *
 * @code
 * TLINFO("Map has %u buckets and %u entries",
 *     tl_map_capacity(map),
 *     tl_map_size(map));
 * @endcode
 */
u32 tl_map_capacity(TLMap* map);

/**
 * @brief Check if map is empty
 *
 * Returns true if the map contains no entries.
 *
 * @param map Map to check
 * @return true if map is empty, false if it contains entries
 *
 * @note Thread-safe - uses internal mutex
 *
 * @see tl_map_size
 *
 * @code
 * if (tl_map_is_empty(map)) {
 *     TLINFO("Map is empty");
 * } else {
 *     process_map(map);
 * }
 * @endcode
 */
b8 tl_map_is_empty(TLMap* map);

/**
 * @brief Clear the map (remove all entries without destroying it)
 *
 * Removes and frees all entries, keys, and values. The map structure remains
 * valid and can be immediately reused.
 *
 * @param map Map to clear
 *
 * @note All TLString keys are destroyed
 * @note All TLList values are destroyed (but not the data they contain)
 * @note Thread-safe - uses internal mutex
 * @note This is faster than destroying and recreating the map
 *
 * @see tl_map_destroy
 * @see tl_map_is_empty
 *
 * @code
 * // Free all data before clearing
 * // (Note: This requires iterating all buckets and entries manually)
 *
 * // Clear the map
 * tl_map_clear(map);
 * @endcode
 */
void tl_map_clear(TLMap* map);

// =================================
// ITERATOR API (Fail-Fast)
// =================================

/**
 * @brief Destroy an iterator and free its memory
 *
 * Frees the iterator structure.
 *
 * @param iterator Iterator to destroy (may be NULL)
 *
 * @note Safe to call with NULL (no-op)
 * @note Does NOT affect the original container
 * @note Does NOT free the data pointed to by items
 *
 * @see tl_list_iterator
 * @see tl_map_keys
 * @see tl_queue_iterator
 * @see tl_pool_iterator
 *
 * @code
 * TLIterator* iter = tl_list_iterator(list);
 * // ... use iterator ...
 * tl_iterator_destroy(iter);
 * @endcode
 */
void tl_iterator_destroy(TLIterator* iterator);

/**
 * @brief Check if iterator has more items
 *
 * Returns true if there are more items to iterate over.
 * Also performs fail-fast check for concurrent modifications.
 *
 * @param iterator Iterator to check
 * @return true if more items available, false otherwise
 *
 * @note Checks for concurrent modification (fail-fast)
 * @note Returns false if iterator is NULL
 * @note Causes FATAL error if container was modified
 *
 * @see tl_iterator_next
 *
 * @code
 * while (tl_iterator_has_next(iter)) {
 *     void* item = tl_iterator_next(iter);
 *     process_item(item);
 * }
 * @endcode
 */
b8 tl_iterator_has_next(const TLIterator* iterator);

/**
 * @brief Get next item from iterator
 *
 * Returns the next item and advances the iterator.
 * Performs fail-fast check for concurrent modifications.
 *
 * @param iterator Iterator to advance
 * @return Next void* pointer, or NULL if no more items or iterator is NULL
 *
 * @note Checks for concurrent modification (fail-fast)
 * @note Returns NULL if iterator is NULL or exhausted
 * @note Causes FATAL error if container was modified
 *
 * @see tl_iterator_has_next
 * @see tl_iterator_rewind
 *
 * @code
 * while (tl_iterator_has_next(iter)) {
 *     void* item = tl_iterator_next(iter);
 *     MyData* data = (MyData*)item;
 *     process(data);
 * }
 * @endcode
 */
void* tl_iterator_next(TLIterator* iterator);

/**
 * @brief Rewind iterator to beginning
 *
 * Resets the iterator to the beginning of the container.
 * Performs fail-fast check for concurrent modifications.
 *
 * @param iterator Iterator to reset
 *
 * @note Resets position to start of container
 * @note Checks for concurrent modification (fail-fast)
 * @note Causes FATAL error if container was modified
 *
 * @see tl_list_iterator
 * @see tl_map_keys
 * @see tl_queue_iterator
 * @see tl_pool_iterator
 *
 * @code
 * TLIterator* iter = tl_list_iterator(list);
 *
 * // First pass
 * while (tl_iterator_has_next(iter)) {
 *     void* item = tl_iterator_next(iter);
 *     first_pass(item);
 * }
 *
 * // Second pass
 * tl_iterator_rewind(iter);
 * while (tl_iterator_has_next(iter)) {
 *     void* item = tl_iterator_next(iter);
 *     second_pass(item);
 * }
 *
 * tl_iterator_destroy(iter);
 * @endcode
 */
void tl_iterator_rewind(TLIterator* iterator);

/**
 * @brief Resynchronize iterator with current container state
 *
 * Re-captures the container's modification count, size, and rewinds to the beginning.
 * This allows reusing a cached iterator even after the container has been modified.
 *
 * @param iterator Iterator to resynchronize
 *
 * @note Thread-safe - acquires container's mutex during resync
 * @note Updates expected_mod_count to current container mod_count
 * @note Updates size to current container size
 * @note Resets iteration position to beginning
 * @note Use this for iterator caching optimization in hot paths
 *
 * @see tl_iterator_rewind
 * @see tl_list_iterator
 * @see tl_map_keys
 * @see tl_queue_iterator
 * @see tl_pool_iterator
 *
 * @code
 * // Cache iterator for reuse (optimization)
 * TLIterator* cached_iter = tl_list_iterator(list);
 *
 * // Use multiple times
 * for (int i = 0; i < 100; ++i) {
 *     // List may have been modified
 *     tl_list_push_back(list, new_item);
 *
 *     // Resync before reuse (updates mod_count and rewinds)
 *     tl_iterator_resync(cached_iter);
 *
 *     while (tl_iterator_has_next(cached_iter)) {
 *         void* item = tl_iterator_next(cached_iter);
 *         process(item);
 *     }
 * }
 *
 * tl_iterator_destroy(cached_iter);
 * @endcode
 */
void tl_iterator_resync(TLIterator* iterator);

/**
 * @brief Get total number of items in iterator
 *
 * Returns the number of items in the container at iterator creation time.
 *
 * @param iterator Iterator to query
 * @return Number of items, or 0 if iterator is NULL
 *
 * @note This is the size at iterator creation time
 * @note Does NOT check for concurrent modification
 *
 * @see tl_list_iterator
 * @see tl_map_keys
 * @see tl_queue_iterator
 * @see tl_pool_iterator
 *
 * @code
 * TLIterator* iter = tl_list_iterator(list);
 * TLINFO("Iterating over %u items", tl_iterator_size(iter));
 *
 * while (tl_iterator_has_next(iter)) {
 *     void* item = tl_iterator_next(iter);
 *     process(item);
 * }
 *
 * tl_iterator_destroy(iter);
 * @endcode
 */
u32 tl_iterator_size(const TLIterator* iterator);

#endif
