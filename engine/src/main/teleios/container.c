/**
 * @file container.c
 * @brief Container module dispatcher
 *
 * This file serves as the central compilation unit for all container implementations.
 * It includes all container-specific .inl files which contain the actual implementations.
 *
 * Included implementations:
 * - Queue: Ring buffer with thread-safe blocking operations
 * - Pool: Pre-allocated object pool with O(1) acquire/release
 * - List: Double linked list with bidirectional traversal
 * - Map: Hash map with TLString keys and TLList* values
 * - Iterator: Fail-fast iterator with snapshot-based traversal
 */

// Include all container implementations
#include "teleios/container/queue.inl"
#include "teleios/container/pool.inl"
#include "teleios/container/list.inl"
#include "teleios/container/map.inl"
#include "teleios/container/iterator.inl"
