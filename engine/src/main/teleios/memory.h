/**
 * @file memory.h
 * @brief Custom memory allocators with leak detection and tagged allocation tracking
 *
 * This module provides a flexible memory allocation system supporting two allocator types:
 * - **LINEAR**: Arena allocator for subsystems with bulk allocation/deallocation
 * - **DYNAMIC**: Heap allocator with individual deallocation and leak detection
 *
 * The memory system uses memory tags to track allocations by subsystem, enabling:
 * - Leak detection and reporting in DYNAMIC allocators
 * - Per-subsystem memory profiling
 * - Debugging of memory issues during development
 *
 * @section allocators Allocator Types
 *
 * **LINEAR Allocator:**
 * - Fast allocation: O(1) allocation, simple pointer increment
 * - No individual deallocation: All memory freed when allocator destroyed
 * - Ideal for subsystems with clear initialization/cleanup phases
 * - Examples: scene loading, frame-based temporary allocations
 *
 * **DYNAMIC Allocator:**
 * - Standard heap allocation with individual deallocation
 * - Tracks all allocations for leak detection
 * - Slightly slower than LINEAR but flexible lifetime management
 * - Examples: threaded resources, objects with variable lifetime
 *
 * @section memory_tags Memory Tags
 * Memory tags are used to classify allocations by subsystem:
 * - `TL_MEMORY_BLOCK` - Generic/untagged blocks
 * - `TL_MEMORY_GRAPHICS` - Graphics subsystem resources
 * - `TL_MEMORY_SERIALIZER` - Serialization (YAML, JSON, etc)
 * - `TL_MEMORY_CONTAINER_*` - Container data structures
 * - `TL_MEMORY_PROFILER` - Profiler state and buffers
 * - `TL_MEMORY_SCENE` - Scene and game object data
 * - `TL_MEMORY_ECS_COMPONENT` - ECS component allocations
 * - `TL_MEMORY_THREAD` - Thread-related allocations
 *
 * @section initialization Initialization
 * Memory system must be initialized before any allocators are created:
 * @code
 * if (!tl_memory_initialize()) {
 *     TLFATAL("Memory system failed to initialize");
 * }
 * @endcode
 *
 * @section usage Usage Examples
 *
 * **Creating a LINEAR allocator:**
 * @code
 * // Create 1 MB arena allocator for scene data
 * TLAllocator* scene_alloc = tl_memory_allocator_create(
 *     TL_MEBI_BYTES(1),
 *     TL_ALLOCATOR_LINEAR
 * );
 *
 * if (scene_alloc == NULL) {
 *     TLERROR("Failed to create scene allocator");
 *     return false;
 * }
 *
 * // Allocate scene objects (fast, O(1))
 * GameEntity* entities = tl_memory_alloc(
 *     scene_alloc,
 *     TL_MEMORY_SCENE,
 *     sizeof(GameEntity) * 1000
 * );
 *
 * // ... use entities ...
 *
 * // Destroy allocator - all memory freed at once
 * tl_memory_allocator_destroy(scene_alloc);
 * @endcode
 *
 * **Creating a DYNAMIC allocator:**
 * @code
 * // Create dynamic allocator for threaded resources
 * TLAllocator* heap = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
 *
 * // Allocate individual objects
 * ThreadContext* ctx = tl_memory_alloc(
 *     heap,
 *     TL_MEMORY_THREAD,
 *     sizeof(ThreadContext)
 * );
 *
 * // ... use context ...
 *
 * // Free individual allocation
 * tl_memory_free(heap, ctx);
 *
 * // Destroy allocator - reports any remaining leaks
 * tl_memory_allocator_destroy(heap);  // May log leak warnings
 * @endcode
 *
 * **Memory set and copy operations:**
 * @code
 * // Fill buffer with pattern (like memset)
 * char buffer[256];
 * tl_memory_set(buffer, 0, sizeof(buffer));
 *
 * // Copy memory (like memcpy)
 * struct Data src = {...};
 * struct Data dst;
 * tl_memory_copy(&dst, &src, sizeof(struct Data));
 * @endcode
 *
 * @note Thread safety: Each thread should use its own allocators. The memory system
 *       is not thread-safe for concurrent allocation from the same allocator.
 *
 * @note Leak detection: DYNAMIC allocators report leaked allocations on destroy.
 *       LINEAR allocators never report leaks (by design).
 *
 * @see tl_platform_initialize - Initializes memory system
 * @see defines.h - Memory size helper macros (TL_MEBI_BYTES, TL_KIBI_BYTES, etc)
 *
 * @author TELEIOS Team
 * @version 0.1.0
 */

#ifndef __TELEIOS_MEMORY__
#define __TELEIOS_MEMORY__

#include "teleios/defines.h"

/**
 * @brief Memory allocation tags for subsystem tracking
 *
 * Used to categorize allocations by subsystem for profiling and leak detection.
 * Each tag represents a major subsystem or data structure type.
 */
typedef enum {
    TL_MEMORY_BLOCK,                    ///< Generic/untagged memory blocks
    TL_MEMORY_GRAPHICS,                 ///< Graphics subsystem (textures, shaders, etc)
    TL_MEMORY_SERIALIZER,               ///< Serialization buffers (YAML, JSON parsing)
    TL_MEMORY_CONTAINER_QUEUE,          ///< Queue container allocations
    TL_MEMORY_CONTAINER_POOL,           ///< Object pool allocations
    TL_MEMORY_CONTAINER_STACK,          ///< Stack container allocations
    TL_MEMORY_CONTAINER_LIST,           ///< Linked list allocations
    TL_MEMORY_CONTAINER_NODE,           ///< Container node structures
    TL_MEMORY_CONTAINER_MAP,            ///< Hash map allocations
    TL_MEMORY_CONTAINER_MAP_ENTRY,      ///< Hash map entry allocations
    TL_MEMORY_CONTAINER_ITERATOR,       ///< Iterator allocations
    TL_MEMORY_STRING,                   ///< String allocations
    TL_MEMORY_ULID,                     ///< ULID identifier allocations
    TL_MEMORY_PROFILER,                 ///< Profiler state and buffers
    TL_MEMORY_SCENE,                    ///< Scene and game object data
    TL_MEMORY_ECS_COMPONENT,            ///< ECS component allocations
    TL_MEMORY_THREAD,                   ///< Thread-related allocations
    TL_MEMORY_MAXIMUM                   ///< Sentinel value for bounds checking
} TLMemoryTag;

/**
 * @brief Allocator type enumeration
 *
 * Specifies the allocation strategy for a memory allocator.
 *
 * @see tl_memory_allocator_create
 */
typedef enum {
    TL_ALLOCATOR_LINEAR,        ///< Arena allocator - bulk allocation/deallocation
    TL_ALLOCATOR_DYNAMIC        ///< Heap allocator - individual deallocation with leak detection
} TLAllocatorType;

/**
 * @brief Opaque allocator handle
 *
 * Represents a memory allocator instance. The actual structure is defined
 * in the implementation files (memory_linear.inl, memory_dynamic.inl).
 */
typedef struct TLAllocator TLAllocator;

/**
 * @brief Initialize the memory system
 *
 * Sets up the memory tracking and default allocators. Must be called before
 * creating any custom allocators.
 *
 * @return true on success, false on failure
 *
 * @note Typically called by tl_platform_initialize()
 *
 * @see tl_memory_terminate
 * @see tl_platform_initialize
 */
b8 tl_memory_initialize(void);

/**
 * @brief Terminate the memory system
 *
 * Cleans up all allocated memory and reports any leaks from DYNAMIC allocators.
 * Should be called after all allocators are destroyed.
 *
 * @return true on success, false on failure
 *
 * @note Typically called by tl_platform_terminate()
 *
 * @see tl_memory_initialize
 * @see tl_platform_terminate
 */
b8 tl_memory_terminate(void);

/**
 * @brief Create a memory allocator
 *
 * Creates a new allocator instance of the specified type. The size parameter
 * interpretation depends on allocator type:
 *
 * - **LINEAR**: Initial arena size in bytes (memory grows if exhausted)
 * - **DYNAMIC**: Ignored (set to 0)
 *
 * @param size Arena size for LINEAR allocator, 0 for DYNAMIC allocator
 * @param type Allocator type (LINEAR or DYNAMIC)
 * @return Pointer to newly created allocator, or NULL on failure
 *
 * @note LINEAR allocators are fast but cannot deallocate individual blocks.
 *       Use for subsystems that allocate once and deallocate everything at shutdown.
 *
 * @note DYNAMIC allocators support individual deallocation and track allocations
 *       for leak detection on destruction.
 *
 * @see tl_memory_allocator_destroy
 * @see tl_memory_alloc
 * @see tl_memory_free
 *
 * @code
 * // Create 4 MB LINEAR arena for frame-based allocations
 * TLAllocator* frame_alloc = tl_memory_allocator_create(
 *     TL_MEBI_BYTES(4),
 *     TL_ALLOCATOR_LINEAR
 * );
 *
 * // Create DYNAMIC allocator for general heap usage
 * TLAllocator* heap = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
 * @endcode
 */
TLAllocator* tl_memory_allocator_create(u32 size, TLAllocatorType type);

/**
 * @brief Destroy a memory allocator and cleanup resources
 *
 * Frees all memory associated with the allocator. For DYNAMIC allocators,
 * reports any leaked allocations to the logger before destruction.
 *
 * For LINEAR allocators, all memory is freed in one operation.
 * For DYNAMIC allocators, ensure all allocated memory has been freed
 * before destruction to avoid leak warnings.
 *
 * @param allocator Allocator to destroy (may be NULL)
 *
 * @note Safe to call with NULL allocator (no-op).
 * @note After destruction, using the allocator is undefined behavior.
 *
 * @see tl_memory_allocator_create
 * @see tl_memory_alloc
 * @see tl_memory_free
 *
 * @code
 * TLAllocator* alloc = tl_memory_allocator_create(
 *     TL_MEBI_BYTES(1),
 *     TL_ALLOCATOR_LINEAR
 * );
 *
 * // ... use allocator ...
 *
 * tl_memory_allocator_destroy(alloc);
 * alloc = NULL;  // Good practice
 * @endcode
 */
void tl_memory_allocator_destroy(TLAllocator* allocator);

/**
 * @brief Allocate memory from an allocator
 *
 * Requests a block of memory from the specified allocator. The allocation
 * is tagged for tracking and leak detection purposes.
 *
 * @param allocator Allocator to use (must not be NULL)
 * @param tag Memory tag for categorization (see TLMemoryTag)
 * @param size Size in bytes to allocate (must not be 0)
 * @return Pointer to allocated memory, or NULL if allocation failed
 *
 * @note The returned memory is not initialized (may contain garbage).
 *       Use tl_memory_set() to zero-initialize if needed.
 *
 * @note LINEAR allocator failures may reallocate the arena (pointer invalidation
 *       for old allocations is possible). Avoid relying on pointer stability.
 *
 * @see tl_memory_allocator_create
 * @see tl_memory_free
 * @see tl_memory_set
 *
 * @code
 * TLAllocator* alloc = tl_memory_allocator_create(
 *     TL_MEBI_BYTES(1),
 *     TL_ALLOCATOR_LINEAR
 * );
 *
 * // Allocate space for 100 entities
 * GameEntity* entities = (GameEntity*)tl_memory_alloc(
 *     alloc,
 *     TL_MEMORY_SCENE,
 *     sizeof(GameEntity) * 100
 * );
 *
 * if (entities == NULL) {
 *     TLERROR("Failed to allocate entities");
 *     return false;
 * }
 *
 * // Initialize memory
 * tl_memory_set(entities, 0, sizeof(GameEntity) * 100);
 * @endcode
 */
void* tl_memory_alloc(TLAllocator* allocator, TLMemoryTag tag, u32 size);

/**
 * @brief Free memory allocated from an allocator
 *
 * Deallocates a block of memory previously allocated with tl_memory_alloc().
 *
 * **For LINEAR allocators:** This is a no-op. Memory cannot be individually
 * freed from LINEAR allocators. Use tl_memory_allocator_destroy() to free
 * all memory at once.
 *
 * **For DYNAMIC allocators:** The memory block is returned to the allocator
 * for reuse. The pointer becomes invalid after this call.
 *
 * @param allocator Allocator that owns the memory
 * @param pointer Pointer to free (may be NULL)
 *
 * @note Safe to call with NULL pointer (no-op).
 * @note Freeing the same pointer twice causes undefined behavior.
 * @note Using the pointer after freeing causes undefined behavior.
 *
 * @see tl_memory_alloc
 * @see tl_memory_allocator_destroy
 *
 * @code
 * TLAllocator* heap = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
 *
 * void* data = tl_memory_alloc(heap, TL_MEMORY_BLOCK, 1024);
 *
 * // ... use data ...
 *
 * tl_memory_free(heap, data);  // Deallocate
 * data = NULL;  // Good practice
 *
 * tl_memory_allocator_destroy(heap);
 * @endcode
 */
void tl_memory_free(TLAllocator* allocator, void* pointer);

/**
 * @brief Fill memory with a repeated byte value
 *
 * Sets all bytes in a memory region to the specified value. Similar to memset().
 *
 * @param target Pointer to memory region to fill
 * @param value Byte value to fill (0-255)
 * @param size Size in bytes to fill
 *
 * @note If target is NULL or size is 0, behavior is undefined.
 *
 * @see tl_memory_copy
 *
 * @code
 * // Zero-initialize a buffer
 * char buffer[256];
 * tl_memory_set(buffer, 0, sizeof(buffer));
 *
 * // Fill with pattern
 * unsigned char pattern[16];
 * tl_memory_set(pattern, 0xAB, sizeof(pattern));
 * @endcode
 */
void tl_memory_set(void *target, i32 value, u32 size);

/**
 * @brief Copy memory from source to destination
 *
 * Copies bytes from source to destination. Similar to memcpy(), but regions
 * must not overlap (use only for non-overlapping regions).
 *
 * @param target Destination memory pointer
 * @param source Source memory pointer
 * @param size Number of bytes to copy
 *
 * @note Behavior is undefined if regions overlap. Use memmove() semantics
 *       not available; ensure source and destination don't overlap.
 * @note If either pointer is NULL or size is 0, behavior is undefined.
 *
 * @see tl_memory_set
 *
 * @code
 * struct Data src = {...};
 * struct Data dst;
 *
 * // Copy structure
 * tl_memory_copy(&dst, &src, sizeof(struct Data));
 *
 * // Copy array
 * int src_array[100] = {...};
 * int dst_array[100];
 * tl_memory_copy(dst_array, src_array, sizeof(src_array));
 * @endcode
 */
void tl_memory_copy(void *target, const void *source, u32 size);

#endif
