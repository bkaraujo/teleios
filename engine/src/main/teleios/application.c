#include "teleios/teleios.h"
#include "teleios/application.h"
#include <GLFW/glfw3.h>

#define FRAME_CAP 250000.0

#include "teleios/application/event.inl"

b8 tl_application_initialize(void) {
    TL_PROFILER_PUSH

    tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, tl_application_handle_window_closed);
    tl_event_subscribe(TL_EVENT_WINDOW_RESTORED, tl_application_handle_window_restored);
    tl_event_subscribe(TL_EVENT_WINDOW_MINIMIZED, tl_application_handle_window_minimized);
    
    TL_PROFILER_POP_WITH(true)
}

b8 tl_application_run(void) {
    TL_PROFILER_PUSH

    TLString* sStep = tl_config_get("teleios.simulation.step");
    const f32 fStep = tl_number_f32_from_string(sStep);
    const f64 STEP = 1000000.0 / (fStep == 0 ? 10 : fStep);

    f64 accumulator = 0.0;

    global->running = true;
    f64 fps_timer = 0.0;
    u64 last_time = tl_time_epoch_micros();
    u64 last_frame_count = 0;
    u64 last_update_count = 0;

    tl_scene_load();

    TLString* script = tl_string_create(global->allocator, "assets/script/test.lua");

    TLDEBUG("Entering main loop")
    glfwShowWindow(tl_window_handler());
    for ( ; global->running ; ) {
        const u64 new_time = tl_time_epoch_micros();
        f64 delta_time = (f64)(new_time - last_time);
        last_time = new_time;
        tl_scene_frame_begin();
        if (!global->suspended) {
            global->update_count++;

            // Cap frame time to prevent spiral of death
            if (delta_time > FRAME_CAP) {
                TLWARN("Frame time %.2f ms exceeded, capping to %.2f ms",  delta_time / 1000.0, FRAME_CAP / 1000.0);
                delta_time = FRAME_CAP;
            }

            accumulator += delta_time;
            while (accumulator >= STEP) {
                tl_scene_step(STEP);
                accumulator -= STEP;
            }

            tl_scene_update(delta_time);
        }

        tl_script_execute(script);
        tl_scene_frame_end();
        tl_input_update();
        glfwPollEvents();

        fps_timer += delta_time;
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
    tl_string_destroy(script);

    TLDEBUG("Finalizando")
    TL_PROFILER_POP_WITH(true)
}

b8 tl_application_terminate(void) {
    TL_PROFILER_PUSH
    
    TL_PROFILER_POP_WITH(true)
}