#ifndef __TEST_FRAMEWORK__
#define __TEST_FRAMEWORK__

#include <stdio.h>
#include <string.h>

// Test result tracking
static int g_tests_passed = 0;
static int g_tests_failed = 0;
static int g_assertions_passed = 0;
static int g_assertions_failed = 0;

// Test suite tracking
static const char* g_current_suite = NULL;
static const char* g_current_test = NULL;

// Color codes for terminal output
#ifdef _WIN32
#define COLOR_GREEN ""
#define COLOR_RED ""
#define COLOR_YELLOW ""
#define COLOR_RESET ""
#else
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RESET "\033[0m"
#endif

// Test suite macros
#define TEST_SUITE_BEGIN(name) \
    do { \
        g_current_suite = name; \
        printf("\n=== Test Suite: %s ===\n", name); \
        fflush(stdout); \
    } while(0)

#define TEST_SUITE_END() \
    do { \
        printf("=== End Suite: %s ===\n", g_current_suite); \
        fflush(stdout); \
        g_current_suite = NULL; \
    } while(0)

// Test case macros
#define TEST_BEGIN(name) \
    do { \
        g_current_test = name; \
        int _test_failures = g_assertions_failed; \
        (void)0

#define TEST_END() \
        if (g_assertions_failed == _test_failures) { \
            g_tests_passed++; \
            printf("  [PASS] %s\n", g_current_test); \
        } else { \
            g_tests_failed++; \
            printf("  [FAIL] %s\n", g_current_test); \
        } \
        fflush(stdout); \
        g_current_test = NULL; \
    } while(0)

// Assertion macros
#define ASSERT_TRUE(condition) \
    do { \
        if (condition) { \
            g_assertions_passed++; \
        } else { \
            g_assertions_failed++; \
            fprintf(stderr, "    ASSERT_TRUE failed: %s (line %d)\n", #condition, __LINE__); \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (!(condition)) { \
            g_assertions_passed++; \
        } else { \
            g_assertions_failed++; \
            fprintf(stderr, "    ASSERT_FALSE failed: %s (line %d)\n", #condition, __LINE__); \
        } \
    } while(0)

#define ASSERT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            g_assertions_passed++; \
        } else { \
            g_assertions_failed++; \
            fprintf(stderr, "    ASSERT_NULL failed: %s (line %d)\n", #ptr, __LINE__); \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            g_assertions_passed++; \
        } else { \
            g_assertions_failed++; \
            fprintf(stderr, "    ASSERT_NOT_NULL failed: %s (line %d)\n", #ptr, __LINE__); \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            g_assertions_passed++; \
        } else { \
            g_assertions_failed++; \
            fprintf(stderr, "    ASSERT_EQ failed: %s == %s (line %d)\n", #expected, #actual, __LINE__); \
        } \
    } while(0)

#define ASSERT_NE(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            g_assertions_passed++; \
        } else { \
            g_assertions_failed++; \
            fprintf(stderr, "    ASSERT_NE failed: %s != %s (line %d)\n", #expected, #actual, __LINE__); \
        } \
    } while(0)

#define ASSERT_GT(a, b) \
    do { \
        if ((a) > (b)) { \
            g_assertions_passed++; \
        } else { \
            g_assertions_failed++; \
            fprintf(stderr, "    ASSERT_GT failed: %s > %s (line %d)\n", #a, #b, __LINE__); \
        } \
    } while(0)

#define ASSERT_GE(a, b) \
    do { \
        if ((a) >= (b)) { \
            g_assertions_passed++; \
        } else { \
            g_assertions_failed++; \
            fprintf(stderr, "    ASSERT_GE failed: %s >= %s (line %d)\n", #a, #b, __LINE__); \
        } \
    } while(0)

#define ASSERT_LT(a, b) \
    do { \
        if ((a) < (b)) { \
            g_assertions_passed++; \
        } else { \
            g_assertions_failed++; \
            fprintf(stderr, "    ASSERT_LT failed: %s < %s (line %d)\n", #a, #b, __LINE__); \
        } \
    } while(0)

#define ASSERT_LE(a, b) \
    do { \
        if ((a) <= (b)) { \
            g_assertions_passed++; \
        } else { \
            g_assertions_failed++; \
            fprintf(stderr, "    ASSERT_LE failed: %s <= %s (line %d)\n", #a, #b, __LINE__); \
        } \
    } while(0)

#define ASSERT_STR_EQ(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) == 0) { \
            g_assertions_passed++; \
        } else { \
            g_assertions_failed++; \
            fprintf(stderr, "    ASSERT_STR_EQ failed: \"%s\" != \"%s\" (line %d)\n", (expected), (actual), __LINE__); \
        } \
    } while(0)

#define ASSERT_FLOAT_EQ(expected, actual, epsilon) \
    do { \
        double _diff = (expected) - (actual); \
        if (_diff < 0) _diff = -_diff; \
        if (_diff <= (epsilon)) { \
            g_assertions_passed++; \
        } else { \
            g_assertions_failed++; \
            fprintf(stderr, "    ASSERT_FLOAT_EQ failed: %f != %f (line %d)\n", (double)(expected), (double)(actual), __LINE__); \
        } \
    } while(0)

// Summary functions
static inline void test_print_summary(void) {
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Tests:      %d passed, %d failed\n", g_tests_passed, g_tests_failed);
    printf("  Assertions: %d passed, %d failed\n", g_assertions_passed, g_assertions_failed);
    printf("========================================\n");

    if (g_tests_failed == 0) {
        printf("All tests passed!\n");
    } else {
        printf("Some tests FAILED!\n");
    }
}

static inline int test_get_exit_code(void) {
    return g_tests_failed > 0 ? 1 : 0;
}

#endif
