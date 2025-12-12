#include "test_framework.h"
#include "teleios/teleios.h"

// Test state for event handlers
static int g_event_handler_called = 0;
static TLEvent g_last_event = {0};
static int g_handler1_calls = 0;
static int g_handler2_calls = 0;

// Simple event handler that records it was called
static TLEventStatus test_handler_available(const TLEvent* event) {
    g_event_handler_called++;
    if (event != NULL) {
        g_last_event = *event;
    }
    return TL_EVENT_AVAILABLE;
}

// Handler that consumes the event
static TLEventStatus test_handler_consumed(const TLEvent* event) {
    (void)event;
    g_event_handler_called++;
    return TL_EVENT_CONSUMED;
}

// First handler in chain
static TLEventStatus test_handler1(const TLEvent* event) {
    (void)event;
    g_handler1_calls++;
    return TL_EVENT_AVAILABLE;
}

// Second handler in chain
static TLEventStatus test_handler2(const TLEvent* event) {
    (void)event;
    g_handler2_calls++;
    return TL_EVENT_AVAILABLE;
}

// Reset test state
static void reset_event_test_state(void) {
    g_event_handler_called = 0;
    tl_memory_set(&g_last_event, 0, sizeof(g_last_event));
    g_handler1_calls = 0;
    g_handler2_calls = 0;
}

void test_event(void) {
    TEST_SUITE_BEGIN("Event");

    // ============================================
    // Event Subscription
    // ============================================

    TEST_BEGIN("tl_event_subscribe");
    {
        reset_event_test_state();

        // Subscribe should return true on success
        b8 result = tl_event_subscribe(TL_EVENT_WINDOW_CREATED, test_handler_available);
        ASSERT_TRUE(result);
    }
    TEST_END();

    TEST_BEGIN("tl_event_subscribe_multiple");
    {
        reset_event_test_state();

        // Can subscribe multiple handlers to same event
        b8 result1 = tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, test_handler1);
        b8 result2 = tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, test_handler2);

        ASSERT_TRUE(result1);
        ASSERT_TRUE(result2);
    }
    TEST_END();

    // ============================================
    // Event Submission
    // ============================================

    TEST_BEGIN("tl_event_submit_no_data");
    {
        reset_event_test_state();

        tl_event_subscribe(TL_EVENT_WINDOW_MINIMIZED, test_handler_available);
        tl_event_submit(TL_EVENT_WINDOW_MINIMIZED, NULL);

        ASSERT_EQ(1, g_event_handler_called);
    }
    TEST_END();

    TEST_BEGIN("tl_event_submit_with_data");
    {
        reset_event_test_state();

        tl_event_subscribe(TL_EVENT_WINDOW_RESIZED, test_handler_available);

        TLEvent event = {0};
        event.i32[0] = 1920;  // width
        event.i32[1] = 1080;  // height

        tl_event_submit(TL_EVENT_WINDOW_RESIZED, &event);

        ASSERT_EQ(1, g_event_handler_called);
        ASSERT_EQ(1920, g_last_event.i32[0]);
        ASSERT_EQ(1080, g_last_event.i32[1]);
    }
    TEST_END();

    // ============================================
    // Event Propagation
    // ============================================

    TEST_BEGIN("event_propagation_available");
    {
        reset_event_test_state();

        // Both handlers return AVAILABLE, so both should be called
        tl_event_subscribe(TL_EVENT_WINDOW_MAXIMIZED, test_handler1);
        tl_event_subscribe(TL_EVENT_WINDOW_MAXIMIZED, test_handler2);

        tl_event_submit(TL_EVENT_WINDOW_MAXIMIZED, NULL);

        ASSERT_EQ(1, g_handler1_calls);
        ASSERT_EQ(1, g_handler2_calls);
    }
    TEST_END();

    TEST_BEGIN("event_propagation_consumed");
    {
        reset_event_test_state();

        // First handler consumes, second should not be called
        tl_event_subscribe(TL_EVENT_WINDOW_RESTORED, test_handler_consumed);
        tl_event_subscribe(TL_EVENT_WINDOW_RESTORED, test_handler2);

        tl_event_submit(TL_EVENT_WINDOW_RESTORED, NULL);

        ASSERT_EQ(1, g_event_handler_called);
        ASSERT_EQ(0, g_handler2_calls);
    }
    TEST_END();

    // ============================================
    // Event Data Types
    // ============================================

    TEST_BEGIN("event_data_i32");
    {
        reset_event_test_state();

        tl_event_subscribe(TL_EVENT_WINDOW_MOVED, test_handler_available);

        TLEvent event = {0};
        event.i32[0] = 100;
        event.i32[1] = 200;
        event.i32[2] = 300;
        event.i32[3] = 400;

        tl_event_submit(TL_EVENT_WINDOW_MOVED, &event);

        ASSERT_EQ(100, g_last_event.i32[0]);
        ASSERT_EQ(200, g_last_event.i32[1]);
        ASSERT_EQ(300, g_last_event.i32[2]);
        ASSERT_EQ(400, g_last_event.i32[3]);
    }
    TEST_END();

    TEST_BEGIN("event_data_f32");
    {
        reset_event_test_state();

        tl_event_subscribe(TL_EVENT_INPUT_CURSOR_MOVED, test_handler_available);

        TLEvent event = {0};
        event.f32[0] = 123.5f;
        event.f32[1] = 456.75f;

        tl_event_submit(TL_EVENT_INPUT_CURSOR_MOVED, &event);

        ASSERT_FLOAT_EQ(123.5f, g_last_event.f32[0], 0.001f);
        ASSERT_FLOAT_EQ(456.75f, g_last_event.f32[1], 0.001f);
    }
    TEST_END();

    TEST_BEGIN("event_data_i64");
    {
        reset_event_test_state();

        tl_event_subscribe(TL_EVENT_INPUT_KEY_PRESSED, test_handler_available);

        TLEvent event = {0};
        event.i64[0] = 9223372036854775807LL;
        event.i64[1] = -1234567890123456789LL;

        tl_event_submit(TL_EVENT_INPUT_KEY_PRESSED, &event);

        ASSERT_EQ(9223372036854775807LL, g_last_event.i64[0]);
        ASSERT_EQ(-1234567890123456789LL, g_last_event.i64[1]);
    }
    TEST_END();

    TEST_BEGIN("event_data_u8");
    {
        reset_event_test_state();

        tl_event_subscribe(TL_EVENT_INPUT_KEY_RELEASED, test_handler_available);

        TLEvent event = {0};
        for (int i = 0; i < 16; i++) {
            event.u8[i] = (u8)i;
        }

        tl_event_submit(TL_EVENT_INPUT_KEY_RELEASED, &event);

        for (int i = 0; i < 16; i++) {
            ASSERT_EQ((u8)i, g_last_event.u8[i]);
        }
    }
    TEST_END();

    // ============================================
    // Event Codes
    // ============================================

    TEST_BEGIN("event_codes_window");
    {
        // Verify window event codes are distinct
        ASSERT_NE(TL_EVENT_WINDOW_CREATED, TL_EVENT_WINDOW_RESIZED);
        ASSERT_NE(TL_EVENT_WINDOW_CLOSED, TL_EVENT_WINDOW_MOVED);
        ASSERT_NE(TL_EVENT_WINDOW_MINIMIZED, TL_EVENT_WINDOW_MAXIMIZED);
        ASSERT_NE(TL_EVENT_WINDOW_FOCUS_GAINED, TL_EVENT_WINDOW_FOCUS_LOST);
    }
    TEST_END();

    TEST_BEGIN("event_codes_input");
    {
        // Verify input event codes are distinct
        ASSERT_NE(TL_EVENT_INPUT_KEY_PRESSED, TL_EVENT_INPUT_KEY_RELEASED);
        ASSERT_NE(TL_EVENT_INPUT_CURSOR_PRESSED, TL_EVENT_INPUT_CURSOR_RELEASED);
        ASSERT_NE(TL_EVENT_INPUT_CURSOR_MOVED, TL_EVENT_INPUT_CURSOR_SCROLLED);
        ASSERT_NE(TL_EVENT_INPUT_CURSOR_ENTERED, TL_EVENT_INPUT_CURSOR_EXITED);
    }
    TEST_END();

    TEST_BEGIN("event_codes_maximum");
    {
        // TL_EVENT_MAXIMUM should be greater than all predefined events
        ASSERT_GT(TL_EVENT_MAXIMUM, TL_EVENT_WINDOW_CREATED);
        ASSERT_GT(TL_EVENT_MAXIMUM, TL_EVENT_INPUT_CURSOR_EXITED);
    }
    TEST_END();

    // ============================================
    // Edge Cases
    // ============================================

    TEST_BEGIN("event_submit_no_handlers");
    {
        reset_event_test_state();

        // Submitting to an event with no handlers should not crash
        // Using an event code that likely has no handlers
        tl_event_submit(TL_EVENT_MAXIMUM - 1, NULL);

        // Should complete without crashing
        ASSERT_TRUE(true);
    }
    TEST_END();

    TEST_SUITE_END();
}
