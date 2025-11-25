#include "teleios/teleios.h"
#include "teleios/application.h"
#include <GLFW/glfw3.h>

// ---------------------------------
// Event Handlers
// ---------------------------------

static TLEventStatus tl_application_handle_window_closed(const TLEvent *event);
static TLEventStatus tl_application_handle_window_restored(const TLEvent *event);
static TLEventStatus tl_application_handle_window_minimized(const TLEvent *event);

b8 tl_application_initialize(void) {
    TL_PROFILER_PUSH

    tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, tl_application_handle_window_closed);
    tl_event_subscribe(TL_EVENT_WINDOW_RESTORED, tl_application_handle_window_restored);
    tl_event_subscribe(TL_EVENT_WINDOW_MINIMIZED, tl_application_handle_window_minimized);
    
    TL_PROFILER_POP_WITH(true)
}

static void tl_application_loop();

b8 tl_application_run(void) {
    TL_PROFILER_PUSH

    global->running = true;
    glfwShowWindow(tl_window_handler());
    tl_application_loop();
    glfwHideWindow(tl_window_handler());
    TL_PROFILER_POP_WITH(true)
}

static void tl_application_loop() {
    TL_PROFILER_PUSH
    
    f64 fps_timer = 0.0;
    u64 last_time = tl_time_epoch_micros();
    u64 last_frame_count = 0;
    u64 last_update_count = 0;

    TLDEBUG("Entering main loop")
    for ( ; global->running ; ) {
        const u64 new_time = tl_time_epoch_micros();
        const f64 delta_time = (f64)(new_time - last_time);
        last_time = new_time;

        fps_timer += delta_time;
        
        glfwPollEvents();           // Poll events on main thread (GLFW requirement)

        if (fps_timer >= ONE_SECOND_MICROS) {

            static char buffer[60];
            snprintf(buffer, sizeof(buffer), "%s FPS: %llu | UPS: %llu",
                tl_string_cstr(tl_window_title()),
                global->frame_count - last_frame_count,
                global->update_count - last_update_count
            );

            glfwSetWindowTitle(tl_window_handler(), buffer);

            last_frame_count = global->frame_count;
            last_update_count = global->update_count;
            fps_timer -= ONE_SECOND_MICROS;
        }
    }

    TLDEBUG("Exiting main loop")
    TL_PROFILER_POP
}

static TLEventStatus tl_application_handle_window_closed(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    global->running = false;
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

static TLEventStatus tl_application_handle_window_restored(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    global->suspended = false;
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

static TLEventStatus tl_application_handle_window_minimized(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    global->suspended = true;
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

b8 tl_application_terminate(void) {
    TL_PROFILER_PUSH
    
    TL_PROFILER_POP_WITH(true)
}