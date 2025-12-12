#include "test_framework.h"
#include "teleios/teleios.h"

void test_container(void) {
    TEST_SUITE_BEGIN("Container");

    TLAllocator* allocator = global->allocator;

    // ============================================
    // Dynamic Array
    // ============================================

    TEST_BEGIN("tl_array_create");
    {
        TLArray* array = tl_array_create(allocator, 10, false);
        ASSERT_NOT_NULL(array);
        ASSERT_EQ(0, tl_array_size(array));
        ASSERT_GE(tl_array_capacity(array), 10);
        ASSERT_TRUE(tl_array_is_empty(array));
        tl_array_destroy(array);
    }
    TEST_END();

    TEST_BEGIN("tl_array_push_pop");
    {
        TLArray* array = tl_array_create(allocator, 4, false);

        int values[] = {10, 20, 30, 40};
        for (int i = 0; i < 4; i++) {
            ASSERT_TRUE(tl_array_push(array, &values[i]));
        }

        ASSERT_EQ(4, tl_array_size(array));
        ASSERT_FALSE(tl_array_is_empty(array));

        // Pop should return in reverse order
        ASSERT_EQ(&values[3], tl_array_pop(array));
        ASSERT_EQ(&values[2], tl_array_pop(array));
        ASSERT_EQ(2, tl_array_size(array));

        tl_array_destroy(array);
    }
    TEST_END();

    TEST_BEGIN("tl_array_get_set");
    {
        TLArray* array = tl_array_create(allocator, 4, false);

        int a = 100, b = 200, c = 300;
        tl_array_push(array, &a);
        tl_array_push(array, &b);

        ASSERT_EQ(&a, tl_array_get(array, 0));
        ASSERT_EQ(&b, tl_array_get(array, 1));

        // Set index 1 to c
        ASSERT_TRUE(tl_array_set(array, 1, &c));
        ASSERT_EQ(&c, tl_array_get(array, 1));

        tl_array_destroy(array);
    }
    TEST_END();

    TEST_BEGIN("tl_array_insert_remove");
    {
        TLArray* array = tl_array_create(allocator, 4, false);

        int a = 1, b = 2, c = 3, d = 4;
        tl_array_push(array, &a);
        tl_array_push(array, &c);

        // Insert b at index 1
        ASSERT_TRUE(tl_array_insert(array, 1, &b));
        ASSERT_EQ(3, tl_array_size(array));
        ASSERT_EQ(&a, tl_array_get(array, 0));
        ASSERT_EQ(&b, tl_array_get(array, 1));
        ASSERT_EQ(&c, tl_array_get(array, 2));

        // Remove b
        ASSERT_TRUE(tl_array_remove(array, &b));
        ASSERT_EQ(2, tl_array_size(array));
        ASSERT_EQ(&a, tl_array_get(array, 0));
        ASSERT_EQ(&c, tl_array_get(array, 1));

        tl_array_destroy(array);
    }
    TEST_END();

    TEST_BEGIN("tl_array_clear");
    {
        TLArray* array = tl_array_create(allocator, 4, false);

        int values[] = {1, 2, 3};
        for (int i = 0; i < 3; i++) {
            tl_array_push(array, &values[i]);
        }

        ASSERT_EQ(3, tl_array_size(array));
        tl_array_clear(array);
        ASSERT_EQ(0, tl_array_size(array));
        ASSERT_TRUE(tl_array_is_empty(array));

        tl_array_destroy(array);
    }
    TEST_END();

    TEST_BEGIN("tl_array_growth");
    {
        TLArray* array = tl_array_create(allocator, 2, false);

        // Push more than initial capacity
        int values[10];
        for (int i = 0; i < 10; i++) {
            values[i] = i;
            ASSERT_TRUE(tl_array_push(array, &values[i]));
        }

        ASSERT_EQ(10, tl_array_size(array));
        ASSERT_GE(tl_array_capacity(array), 10);

        tl_array_destroy(array);
    }
    TEST_END();

    TEST_BEGIN("tl_array_iterator");
    {
        TLArray* array = tl_array_create(allocator, 4, false);

        int values[] = {10, 20, 30};
        for (int i = 0; i < 3; i++) {
            tl_array_push(array, &values[i]);
        }

        TLIterator* iter = tl_array_iterator(array);
        ASSERT_NOT_NULL(iter);
        ASSERT_EQ(3, tl_iterator_size(iter));

        int count = 0;
        while (tl_iterator_has_next(iter)) {
            int* val = (int*)tl_iterator_next(iter);
            ASSERT_EQ(&values[count], val);
            count++;
        }
        ASSERT_EQ(3, count);

        tl_iterator_destroy(iter);
        tl_array_destroy(array);
    }
    TEST_END();

    // ============================================
    // Queue
    // ============================================

    TEST_BEGIN("tl_queue_create");
    {
        TLQueue* queue = tl_queue_create(allocator, 10, false);
        ASSERT_NOT_NULL(queue);
        ASSERT_EQ(0, tl_queue_size(queue));
        ASSERT_EQ(10, tl_queue_capacity(queue));
        ASSERT_TRUE(tl_queue_is_empty(queue));
        ASSERT_FALSE(tl_queue_is_full(queue));
        tl_queue_destroy(queue);
    }
    TEST_END();

    TEST_BEGIN("tl_queue_offer_pop");
    {
        TLQueue* queue = tl_queue_create(allocator, 4, false);

        int a = 1, b = 2, c = 3;
        tl_queue_offer(queue, &a);
        tl_queue_offer(queue, &b);
        tl_queue_offer(queue, &c);

        ASSERT_EQ(3, tl_queue_size(queue));

        // FIFO order
        ASSERT_EQ(&a, tl_queue_pop(queue));
        ASSERT_EQ(&b, tl_queue_pop(queue));
        ASSERT_EQ(&c, tl_queue_pop(queue));
        ASSERT_TRUE(tl_queue_is_empty(queue));

        tl_queue_destroy(queue);
    }
    TEST_END();

    TEST_BEGIN("tl_queue_peek");
    {
        TLQueue* queue = tl_queue_create(allocator, 4, false);

        int a = 100;
        tl_queue_offer(queue, &a);

        // Peek doesn't remove
        ASSERT_EQ(&a, tl_queue_peek(queue));
        ASSERT_EQ(&a, tl_queue_peek(queue));
        ASSERT_EQ(1, tl_queue_size(queue));

        tl_queue_destroy(queue);
    }
    TEST_END();

    TEST_BEGIN("tl_queue_full");
    {
        TLQueue* queue = tl_queue_create(allocator, 3, false);

        int a = 1, b = 2, c = 3;
        tl_queue_offer(queue, &a);
        tl_queue_offer(queue, &b);
        tl_queue_offer(queue, &c);

        ASSERT_TRUE(tl_queue_is_full(queue));
        ASSERT_EQ(3, tl_queue_size(queue));

        tl_queue_destroy(queue);
    }
    TEST_END();

    TEST_BEGIN("tl_queue_clear");
    {
        TLQueue* queue = tl_queue_create(allocator, 4, false);

        int a = 1, b = 2;
        tl_queue_offer(queue, &a);
        tl_queue_offer(queue, &b);

        tl_queue_clear(queue);
        ASSERT_TRUE(tl_queue_is_empty(queue));
        ASSERT_EQ(0, tl_queue_size(queue));

        tl_queue_destroy(queue);
    }
    TEST_END();

    // ============================================
    // Object Pool
    // ============================================

    TEST_BEGIN("tl_pool_create");
    {
        TLObjectPool* pool = tl_pool_create(allocator, sizeof(int), 10, false);
        ASSERT_NOT_NULL(pool);
        ASSERT_EQ(10, tl_pool_capacity(pool));
        ASSERT_EQ(10, tl_pool_available(pool));
        ASSERT_EQ(0, tl_pool_in_use(pool));
        tl_pool_destroy(pool);
    }
    TEST_END();

    TEST_BEGIN("tl_pool_acquire_release");
    {
        TLObjectPool* pool = tl_pool_create(allocator, sizeof(int), 4, false);

        int* obj1 = (int*)tl_pool_acquire(pool);
        ASSERT_NOT_NULL(obj1);
        ASSERT_EQ(3, tl_pool_available(pool));
        ASSERT_EQ(1, tl_pool_in_use(pool));

        *obj1 = 42;

        int* obj2 = (int*)tl_pool_acquire(pool);
        ASSERT_NOT_NULL(obj2);
        ASSERT_EQ(2, tl_pool_available(pool));

        tl_pool_release(pool, obj1);
        ASSERT_EQ(3, tl_pool_available(pool));
        ASSERT_EQ(1, tl_pool_in_use(pool));

        tl_pool_release(pool, obj2);
        ASSERT_EQ(4, tl_pool_available(pool));

        tl_pool_destroy(pool);
    }
    TEST_END();

    TEST_BEGIN("tl_pool_exhaust");
    {
        TLObjectPool* pool = tl_pool_create(allocator, sizeof(int), 2, false);

        void* obj1 = tl_pool_acquire(pool);
        void* obj2 = tl_pool_acquire(pool);
        ASSERT_NOT_NULL(obj1);
        ASSERT_NOT_NULL(obj2);

        // Pool exhausted
        void* obj3 = tl_pool_acquire(pool);
        ASSERT_NULL(obj3);

        tl_pool_release(pool, obj1);
        tl_pool_release(pool, obj2);
        tl_pool_destroy(pool);
    }
    TEST_END();

    TEST_BEGIN("tl_pool_reset");
    {
        TLObjectPool* pool = tl_pool_create(allocator, sizeof(int), 4, false);

        tl_pool_acquire(pool);
        tl_pool_acquire(pool);
        ASSERT_EQ(2, tl_pool_in_use(pool));

        tl_pool_reset(pool);
        ASSERT_EQ(0, tl_pool_in_use(pool));
        ASSERT_EQ(4, tl_pool_available(pool));

        tl_pool_destroy(pool);
    }
    TEST_END();

    // ============================================
    // Double Linked List
    // ============================================

    TEST_BEGIN("tl_list_create");
    {
        TLList* list = tl_list_create(allocator, false);
        ASSERT_NOT_NULL(list);
        ASSERT_EQ(0, tl_list_size(list));
        ASSERT_TRUE(tl_list_is_empty(list));
        tl_list_destroy(list);
    }
    TEST_END();

    TEST_BEGIN("tl_list_push_front_back");
    {
        TLList* list = tl_list_create(allocator, false);

        int a = 1, b = 2, c = 3;
        tl_list_push_front(list, &a);  // [a]
        tl_list_push_back(list, &b);   // [a, b]
        tl_list_push_front(list, &c);  // [c, a, b]

        ASSERT_EQ(3, tl_list_size(list));
        ASSERT_EQ(&c, tl_list_front(list));
        ASSERT_EQ(&b, tl_list_back(list));

        tl_list_destroy(list);
    }
    TEST_END();

    TEST_BEGIN("tl_list_pop_front_back");
    {
        TLList* list = tl_list_create(allocator, false);

        int a = 1, b = 2, c = 3;
        tl_list_push_back(list, &a);
        tl_list_push_back(list, &b);
        tl_list_push_back(list, &c);

        ASSERT_EQ(&a, tl_list_pop_front(list));
        ASSERT_EQ(&c, tl_list_pop_back(list));
        ASSERT_EQ(1, tl_list_size(list));
        ASSERT_EQ(&b, tl_list_front(list));

        tl_list_destroy(list);
    }
    TEST_END();

    TEST_BEGIN("tl_list_clear");
    {
        TLList* list = tl_list_create(allocator, false);

        int a = 1, b = 2;
        tl_list_push_back(list, &a);
        tl_list_push_back(list, &b);

        tl_list_clear(list);
        ASSERT_TRUE(tl_list_is_empty(list));
        ASSERT_EQ(0, tl_list_size(list));

        tl_list_destroy(list);
    }
    TEST_END();

    TEST_BEGIN("tl_list_iterator");
    {
        TLList* list = tl_list_create(allocator, false);

        int values[] = {10, 20, 30};
        for (int i = 0; i < 3; i++) {
            tl_list_push_back(list, &values[i]);
        }

        TLIterator* iter = tl_list_iterator(list);
        ASSERT_NOT_NULL(iter);

        int count = 0;
        while (tl_iterator_has_next(iter)) {
            int* val = (int*)tl_iterator_next(iter);
            ASSERT_EQ(&values[count], val);
            count++;
        }
        ASSERT_EQ(3, count);

        tl_iterator_destroy(iter);
        tl_list_destroy(list);
    }
    TEST_END();

    // ============================================
    // HashMap
    // ============================================

    TEST_BEGIN("tl_map_create");
    {
        TLMap* map = tl_map_create(allocator, 16, false);
        ASSERT_NOT_NULL(map);
        ASSERT_EQ(0, tl_map_size(map));
        ASSERT_TRUE(tl_map_is_empty(map));
        tl_map_destroy(map);
    }
    TEST_END();

    TEST_BEGIN("tl_map_put_get");
    {
        TLMap* map = tl_map_create(allocator, 16, false);

        TLString* key1 = tl_string_create(allocator, "key1");
        TLString* key2 = tl_string_create(allocator, "key2");
        int value1 = 100, value2 = 200;

        tl_map_put(map, key1, &value1);
        tl_map_put(map, key2, &value2);

        ASSERT_EQ(2, tl_map_size(map));
        ASSERT_TRUE(tl_map_contains(map, key1));
        ASSERT_TRUE(tl_map_contains(map, key2));

        TLList* list1 = tl_map_get(map, key1);
        ASSERT_NOT_NULL(list1);
        ASSERT_EQ(&value1, tl_list_front(list1));

        tl_string_destroy(key1);
        tl_string_destroy(key2);
        tl_map_destroy(map);
    }
    TEST_END();

    TEST_BEGIN("tl_map_get_or_create");
    {
        TLMap* map = tl_map_create(allocator, 16, false);

        TLString* key = tl_string_create(allocator, "newkey");

        // Should create new list
        TLList* list = tl_map_get_or_create(map, key);
        ASSERT_NOT_NULL(list);
        ASSERT_TRUE(tl_list_is_empty(list));
        ASSERT_EQ(1, tl_map_size(map));

        // Should return same list
        TLList* list2 = tl_map_get_or_create(map, key);
        ASSERT_EQ(list, list2);
        ASSERT_EQ(1, tl_map_size(map));

        tl_string_destroy(key);
        tl_map_destroy(map);
    }
    TEST_END();

    TEST_BEGIN("tl_map_remove");
    {
        TLMap* map = tl_map_create(allocator, 16, false);

        TLString* key = tl_string_create(allocator, "toremove");
        int value = 42;

        tl_map_put(map, key, &value);
        ASSERT_EQ(1, tl_map_size(map));

        TLList* removed = tl_map_remove(map, key);
        ASSERT_NOT_NULL(removed);
        ASSERT_EQ(0, tl_map_size(map));
        ASSERT_FALSE(tl_map_contains(map, key));

        tl_list_destroy(removed);
        tl_string_destroy(key);
        tl_map_destroy(map);
    }
    TEST_END();

    TEST_BEGIN("tl_map_clear");
    {
        TLMap* map = tl_map_create(allocator, 16, false);

        TLString* key1 = tl_string_create(allocator, "a");
        TLString* key2 = tl_string_create(allocator, "b");
        int v1 = 1, v2 = 2;

        tl_map_put(map, key1, &v1);
        tl_map_put(map, key2, &v2);

        tl_map_clear(map);
        ASSERT_TRUE(tl_map_is_empty(map));
        ASSERT_EQ(0, tl_map_size(map));

        tl_string_destroy(key1);
        tl_string_destroy(key2);
        tl_map_destroy(map);
    }
    TEST_END();

    // ============================================
    // Iterator
    // ============================================

    TEST_BEGIN("tl_iterator_rewind");
    {
        TLArray* array = tl_array_create(allocator, 4, false);

        int a = 1, b = 2;
        tl_array_push(array, &a);
        tl_array_push(array, &b);

        TLIterator* iter = tl_array_iterator(array);

        // First iteration
        while (tl_iterator_has_next(iter)) {
            tl_iterator_next(iter);
        }

        // Rewind and iterate again
        tl_iterator_rewind(iter);
        ASSERT_TRUE(tl_iterator_has_next(iter));
        ASSERT_EQ(&a, tl_iterator_next(iter));

        tl_iterator_destroy(iter);
        tl_array_destroy(array);
    }
    TEST_END();

    // ============================================
    // Thread Safety
    // ============================================

    TEST_BEGIN("thread_safe_array");
    {
        TLArray* array = tl_array_create(allocator, 4, true);
        ASSERT_NOT_NULL(array);

        int a = 1;
        ASSERT_TRUE(tl_array_push(array, &a));
        ASSERT_EQ(&a, tl_array_get(array, 0));

        tl_array_destroy(array);
    }
    TEST_END();

    TEST_BEGIN("thread_safe_queue");
    {
        TLQueue* queue = tl_queue_create(allocator, 4, true);
        ASSERT_NOT_NULL(queue);

        int a = 1;
        tl_queue_offer(queue, &a);
        ASSERT_EQ(&a, tl_queue_pop(queue));

        tl_queue_destroy(queue);
    }
    TEST_END();

    TEST_BEGIN("thread_safe_list");
    {
        TLList* list = tl_list_create(allocator, true);
        ASSERT_NOT_NULL(list);

        int a = 1;
        tl_list_push_back(list, &a);
        ASSERT_EQ(&a, tl_list_pop_back(list));

        tl_list_destroy(list);
    }
    TEST_END();

    TEST_BEGIN("thread_safe_map");
    {
        TLMap* map = tl_map_create(allocator, 8, true);
        ASSERT_NOT_NULL(map);

        TLString* key = tl_string_create(allocator, "test");
        int value = 42;
        tl_map_put(map, key, &value);
        ASSERT_TRUE(tl_map_contains(map, key));

        tl_string_destroy(key);
        tl_map_destroy(map);
    }
    TEST_END();

    TEST_SUITE_END();
}
