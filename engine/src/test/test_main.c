#include "test_framework.h"
#include "teleios/teleios.h"
#include <stdio.h>
#include <string.h>

// Forward declarations of test functions
extern void test_strings(void);
extern void test_memory(void);
extern void test_container(void);
extern void test_number(void);
extern void test_event(void);
extern void test_logger(void);

// Legacy test - can be removed if desired
void test_filesystem(void) {
    TEST_SUITE_BEGIN("Filesystem");

    // Create a temporary test file
    const char* test_filename = "test_file.txt";
    const char* test_content = "Hello, Teleios Filesystem!";
    FILE* f = fopen(test_filename, "w");
    if (f) {
        fprintf(f, "%s", test_content);
        fclose(f);
    } else {
        fprintf(stderr, "Failed to create test file\n");
        return;
    }

    TLString* path = tl_string_create(global->allocator, test_filename);

    TEST_BEGIN("tl_filesystem_exists");
    {
        ASSERT_TRUE(tl_filesystem_exists(path));
    }
    TEST_END();

    TEST_BEGIN("tl_filesystem_size");
    {
        u64 size = tl_filesystem_size(path);
        ASSERT_EQ(strlen(test_content), size);
    }
    TEST_END();

    TEST_BEGIN("tl_filesystem_read");
    {
        TLString* content = tl_filesystem_read(path);
        ASSERT_NOT_NULL(content);
        ASSERT_STR_EQ(test_content, tl_string_cstr(content));
        tl_string_destroy(content);
    }
    TEST_END();

    // Cleanup
    tl_string_destroy(path);
    remove(test_filename);

    TEST_SUITE_END();
}

int main(void) {
    // Disable output buffering for crash debugging
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("========================================\n");
    printf("   TELEIOS Engine Unit Tests\n");
    printf("========================================\n");

    // Initialize global state
    TLGlobal g = {0};
    global = &g;

    // Initialize platform
    if (!tl_platform_initialize()) {
        fprintf(stderr, "Failed to initialize platform\n");
        return 1;
    }

    // Run all test suites
    test_logger();
    test_memory();
    test_strings();
    test_number();
    test_container();
    test_event();
    test_filesystem();

    // Print summary
    test_print_summary();

    // Cleanup
    tl_platform_terminate();

    return test_get_exit_code();
}
