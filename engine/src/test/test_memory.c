#include "test_framework.h"
#include "teleios/teleios.h"

void test_memory(void) {
    TEST_SUITE_BEGIN("Memory");

    // ============================================
    // Linear Allocator
    // ============================================

    TEST_BEGIN("linear_allocator_create");
    {
        TLAllocator* alloc = tl_memory_allocator_create(TL_KIBI_BYTES(4), TL_ALLOCATOR_LINEAR);
        ASSERT_NOT_NULL(alloc);
        tl_memory_allocator_destroy(alloc);
    }
    TEST_END();

    TEST_BEGIN("linear_allocator_alloc");
    {
        TLAllocator* alloc = tl_memory_allocator_create(TL_KIBI_BYTES(4), TL_ALLOCATOR_LINEAR);
        ASSERT_NOT_NULL(alloc);

        void* ptr1 = tl_memory_alloc(alloc, TL_MEMORY_BLOCK, 64);
        ASSERT_NOT_NULL(ptr1);

        void* ptr2 = tl_memory_alloc(alloc, TL_MEMORY_BLOCK, 128);
        ASSERT_NOT_NULL(ptr2);

        // Linear allocator should return different pointers
        ASSERT_NE(ptr1, ptr2);

        tl_memory_allocator_destroy(alloc);
    }
    TEST_END();

    TEST_BEGIN("linear_allocator_multiple_allocations");
    {
        TLAllocator* alloc = tl_memory_allocator_create(TL_KIBI_BYTES(4), TL_ALLOCATOR_LINEAR);

        // Allocate multiple blocks
        for (int i = 0; i < 10; i++) {
            void* ptr = tl_memory_alloc(alloc, TL_MEMORY_BLOCK, 100);
            ASSERT_NOT_NULL(ptr);
        }

        tl_memory_allocator_destroy(alloc);
    }
    TEST_END();

    // ============================================
    // Dynamic Allocator
    // ============================================

    TEST_BEGIN("dynamic_allocator_create");
    {
        TLAllocator* alloc = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
        ASSERT_NOT_NULL(alloc);
        tl_memory_allocator_destroy(alloc);
    }
    TEST_END();

    TEST_BEGIN("dynamic_allocator_alloc_free");
    {
        TLAllocator* alloc = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
        ASSERT_NOT_NULL(alloc);

        void* ptr = tl_memory_alloc(alloc, TL_MEMORY_BLOCK, 256);
        ASSERT_NOT_NULL(ptr);

        tl_memory_free(alloc, ptr);
        tl_memory_allocator_destroy(alloc);
    }
    TEST_END();

    TEST_BEGIN("dynamic_allocator_multiple_alloc_free");
    {
        TLAllocator* alloc = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);

        void* ptrs[10];
        for (int i = 0; i < 10; i++) {
            ptrs[i] = tl_memory_alloc(alloc, TL_MEMORY_BLOCK, 64);
            ASSERT_NOT_NULL(ptrs[i]);
        }

        // Free in reverse order
        for (int i = 9; i >= 0; i--) {
            tl_memory_free(alloc, ptrs[i]);
        }

        tl_memory_allocator_destroy(alloc);
    }
    TEST_END();

    // ============================================
    // Memory Operations
    // ============================================

    TEST_BEGIN("tl_memory_set");
    {
        u8 buffer[16];
        tl_memory_set(buffer, 0xAB, sizeof(buffer));

        for (int i = 0; i < 16; i++) {
            ASSERT_EQ(0xAB, buffer[i]);
        }
    }
    TEST_END();

    TEST_BEGIN("tl_memory_copy");
    {
        u8 src[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        u8 dst[8] = {0};

        tl_memory_copy(dst, src, sizeof(src));

        for (int i = 0; i < 8; i++) {
            ASSERT_EQ(src[i], dst[i]);
        }
    }
    TEST_END();

    TEST_BEGIN("tl_memory_move_non_overlapping");
    {
        u8 src[4] = {1, 2, 3, 4};
        u8 dst[4] = {0};

        tl_memory_move(dst, src, sizeof(src));

        for (int i = 0; i < 4; i++) {
            ASSERT_EQ(src[i], dst[i]);
        }
    }
    TEST_END();

    TEST_BEGIN("tl_memory_move_overlapping");
    {
        u8 buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

        // Shift right: copy [0-3] to [2-5]
        tl_memory_move(&buffer[2], &buffer[0], 4);

        ASSERT_EQ(1, buffer[0]);
        ASSERT_EQ(2, buffer[1]);
        ASSERT_EQ(1, buffer[2]);
        ASSERT_EQ(2, buffer[3]);
        ASSERT_EQ(3, buffer[4]);
        ASSERT_EQ(4, buffer[5]);
    }
    TEST_END();

    // ============================================
    // Memory Tags
    // ============================================

    TEST_BEGIN("memory_tags_usage");
    {
        TLAllocator* alloc = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);

        // Allocate with different tags
        void* graphics = tl_memory_alloc(alloc, TL_MEMORY_GRAPHICS, 64);
        void* string = tl_memory_alloc(alloc, TL_MEMORY_STRING, 32);
        void* container = tl_memory_alloc(alloc, TL_MEMORY_CONTAINER_ARRAY, 128);

        ASSERT_NOT_NULL(graphics);
        ASSERT_NOT_NULL(string);
        ASSERT_NOT_NULL(container);

        tl_memory_free(alloc, graphics);
        tl_memory_free(alloc, string);
        tl_memory_free(alloc, container);

        tl_memory_allocator_destroy(alloc);
    }
    TEST_END();

    TEST_SUITE_END();
}
