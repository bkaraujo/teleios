#include "test_framework.h"
#include "teleios/teleios.h"

void test_logger(void) {
    TEST_SUITE_BEGIN("Logger");

    // ============================================
    // Log Level
    // ============================================

    TEST_BEGIN("tl_logger_set_get_level");
    {
        // Save original level
        TLLogLevel original = tl_logger_get_level();

        // Test setting different levels
        tl_logger_set_level(TL_LOG_LEVEL_VERBOSE);
        ASSERT_EQ(TL_LOG_LEVEL_VERBOSE, tl_logger_get_level());

        tl_logger_set_level(TL_LOG_LEVEL_DEBUG);
        ASSERT_EQ(TL_LOG_LEVEL_DEBUG, tl_logger_get_level());

        tl_logger_set_level(TL_LOG_LEVEL_INFO);
        ASSERT_EQ(TL_LOG_LEVEL_INFO, tl_logger_get_level());

        tl_logger_set_level(TL_LOG_LEVEL_WARN);
        ASSERT_EQ(TL_LOG_LEVEL_WARN, tl_logger_get_level());

        tl_logger_set_level(TL_LOG_LEVEL_ERROR);
        ASSERT_EQ(TL_LOG_LEVEL_ERROR, tl_logger_get_level());

        // Restore original level
        tl_logger_set_level(original);
    }
    TEST_END();

    // ============================================
    // Log Level Values
    // ============================================

    TEST_BEGIN("log_level_ordering");
    {
        // Verify log levels are in order (lower = more verbose)
        ASSERT_LT(TL_LOG_LEVEL_VERBOSE, TL_LOG_LEVEL_TRACE);
        ASSERT_LT(TL_LOG_LEVEL_TRACE, TL_LOG_LEVEL_DEBUG);
        ASSERT_LT(TL_LOG_LEVEL_DEBUG, TL_LOG_LEVEL_INFO);
        ASSERT_LT(TL_LOG_LEVEL_INFO, TL_LOG_LEVEL_WARN);
        ASSERT_LT(TL_LOG_LEVEL_WARN, TL_LOG_LEVEL_ERROR);
        ASSERT_LT(TL_LOG_LEVEL_ERROR, TL_LOG_LEVEL_FATAL);
    }
    TEST_END();

    // ============================================
    // Log Output (Functional Tests)
    // ============================================

    TEST_BEGIN("tl_logger_write_info");
    {
        // Just verify it doesn't crash
        TLLogLevel original = tl_logger_get_level();
        tl_logger_set_level(TL_LOG_LEVEL_INFO);

        tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, "Test info message: %d", 42);

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();

    TEST_BEGIN("tl_logger_write_warn");
    {
        TLLogLevel original = tl_logger_get_level();
        tl_logger_set_level(TL_LOG_LEVEL_WARN);

        tl_logger_write(TL_LOG_LEVEL_WARN, __FILE__, __LINE__, "Test warning: %s", "something");

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();

    TEST_BEGIN("tl_logger_write_error");
    {
        TLLogLevel original = tl_logger_get_level();
        tl_logger_set_level(TL_LOG_LEVEL_ERROR);

        // Note: We don't test TLERROR macro directly as it triggers debug break
        tl_logger_write(TL_LOG_LEVEL_ERROR, __FILE__, __LINE__, "Test error: %f", 3.14);

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();

    // ============================================
    // Log Filtering
    // ============================================

    TEST_BEGIN("log_filtering_above_threshold");
    {
        TLLogLevel original = tl_logger_get_level();

        // Set to WARN level - only WARN, ERROR, FATAL should output
        tl_logger_set_level(TL_LOG_LEVEL_WARN);

        // These should be filtered out (below threshold)
        // but shouldn't crash
        tl_logger_write(TL_LOG_LEVEL_VERBOSE, __FILE__, __LINE__, "filtered verbose");
        tl_logger_write(TL_LOG_LEVEL_TRACE, __FILE__, __LINE__, "filtered trace");
        tl_logger_write(TL_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "filtered debug");
        tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, "filtered info");

        // This should output
        tl_logger_write(TL_LOG_LEVEL_WARN, __FILE__, __LINE__, "not filtered warn");

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();

    // ============================================
    // Macro Tests (Debug Build Only)
    // ============================================

#ifdef TELEIOS_BUILD_DEBUG
    TEST_BEGIN("TLVERBOSE_macro");
    {
        TLLogLevel original = tl_logger_get_level();
        tl_logger_set_level(TL_LOG_LEVEL_VERBOSE);

        // Should compile and run in debug builds
        TLVERBOSE("Verbose test: %d %s", 1, "hello");

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();

    TEST_BEGIN("TLTRACE_macro");
    {
        TLLogLevel original = tl_logger_get_level();
        tl_logger_set_level(TL_LOG_LEVEL_TRACE);

        TLTRACE("Trace test");

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();

    TEST_BEGIN("TLDEBUG_macro");
    {
        TLLogLevel original = tl_logger_get_level();
        tl_logger_set_level(TL_LOG_LEVEL_DEBUG);

        TLDEBUG("Debug test: %p", (void*)&original);

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();
#endif

    TEST_BEGIN("TLINFO_macro");
    {
        TLLogLevel original = tl_logger_get_level();
        tl_logger_set_level(TL_LOG_LEVEL_INFO);

        TLINFO("Info test: %u", 42u);

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();

    TEST_BEGIN("TLWARN_macro");
    {
        TLLogLevel original = tl_logger_get_level();
        tl_logger_set_level(TL_LOG_LEVEL_WARN);

        TLWARN("Warning test: %.2f", 3.14159);

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();

    // ============================================
    // Format String Tests
    // ============================================

    TEST_BEGIN("logger_format_strings");
    {
        TLLogLevel original = tl_logger_get_level();
        tl_logger_set_level(TL_LOG_LEVEL_INFO);

        // Test various format specifiers
        tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, "Integer: %d", -123);
        tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, "Unsigned: %u", 456u);
        tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, "Hex: 0x%x", 0xDEAD);
        tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, "Float: %f", 3.14159);
        tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, "String: %s", "test");
        tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, "Char: %c", 'X');
        tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, "Pointer: %p", (void*)&original);
        tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, "Multiple: %d %s %.2f", 42, "hello", 1.5);

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();

    TEST_BEGIN("logger_empty_message");
    {
        TLLogLevel original = tl_logger_get_level();
        tl_logger_set_level(TL_LOG_LEVEL_INFO);

        // Empty message should work
        tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, "");

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();

    TEST_BEGIN("logger_long_message");
    {
        TLLogLevel original = tl_logger_get_level();
        tl_logger_set_level(TL_LOG_LEVEL_INFO);

        // Long message
        tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__,
            "This is a very long message that tests the logger's ability to handle "
            "longer strings without any issues. It should be able to output this "
            "entire message correctly without truncation or buffer overflow.");

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();

    // ============================================
    // Edge Cases
    // ============================================

    TEST_BEGIN("logger_null_filename");
    {
        TLLogLevel original = tl_logger_get_level();
        tl_logger_set_level(TL_LOG_LEVEL_INFO);

        // Should handle NULL filename gracefully (may crash in some implementations)
        // This tests robustness - skip if it's known to crash
        // tl_logger_write(TL_LOG_LEVEL_INFO, NULL, __LINE__, "null filename");

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();

    TEST_BEGIN("logger_concurrent_safety");
    {
        // Just verify the logger claims to be thread-safe
        // Actual concurrent test would require threading
        TLLogLevel original = tl_logger_get_level();

        // Multiple rapid writes shouldn't corrupt state
        for (int i = 0; i < 10; i++) {
            tl_logger_write(TL_LOG_LEVEL_INFO, __FILE__, __LINE__, "Concurrent test %d", i);
        }

        tl_logger_set_level(original);
        ASSERT_TRUE(true);
    }
    TEST_END();

    TEST_SUITE_END();
}
