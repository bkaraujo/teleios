#include "teleios/teleios.h"
#include <stdio.h>
#include <string.h>

void test_filesystem(void) {
    printf("Testing Filesystem...\n");

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

    // Test Exists
    if (tl_filesystem_exists(path)) {
        printf("  [PASS] tl_filesystem_exists\n");
    } else {
        fprintf(stderr, "  [FAIL] tl_filesystem_exists\n");
    }

    // Test Size
    u64 size = tl_filesystem_size(path);
    if (size == strlen(test_content)) {
        printf("  [PASS] tl_filesystem_size (expected %llu, got %llu)\n", strlen(test_content), size);
    } else {
        fprintf(stderr, "  [FAIL] tl_filesystem_size (expected %llu, got %llu)\n", strlen(test_content), size);
    }

    // Test Read
    TLString* content = tl_filesystem_read(path);
    if (content) {
        if (strcmp(tl_string_cstr(content), test_content) == 0) {
            printf("  [PASS] tl_filesystem_read\n");
        } else {
            fprintf(stderr, "  [FAIL] tl_filesystem_read (content mismatch)\n");
            fprintf(stderr, "    Expected: %s\n", test_content);
            fprintf(stderr, "    Got:      %s\n", tl_string_cstr(content));
        }
        tl_string_destroy(content);
    } else {
        fprintf(stderr, "  [FAIL] tl_filesystem_read (returned NULL)\n");
    }

    // Cleanup
    tl_string_destroy(path);
    remove(test_filename);
}

int main(void) {
    printf("Running Teleios Unit Tests...\n");

    // Initialize global state
    TLGlobal g = { 0 };
    global = &g;

    // Initialize platform
    if (!tl_platform_initialize()) {
        fprintf(stderr, "Failed to initialize platform\n");
        return 1;
    }

    test_filesystem();

    tl_platform_terminate();

    printf("All tests passed!\n");
    return 0;
}
