#include "teleios/teleios.h"
#include "teleios/application.h"
#include <GLFW/glfw3.h>

b8 tl_application_initialize(void) {
    TL_PROFILER_PUSH
    TLTRACE("tl_application_initialize()")
    TL_PROFILER_POP_WITH(true)
}

b8 tl_application_run(void) {
    TL_PROFILER_PUSH
    TLTRACE("tl_application_run()")

    const f64 STEP = 1000000.0 / 60.0;  // ~16666.67 µs
    const f64 FRAME_CAP = 250000.0;  // Cap at 250ms to prevent spiral of death

    f64 accumulator = 0.0;
    u64 last_time = tl_time_epoch_micros();

    // FPS/UPS counters
    u64 frame_count = 0;
    u64 update_count = 0;
    f64 fps_timer = 0.0;

    while (!glfwWindowShouldClose(tl_window_handler())) {
        const u64 new_time = tl_time_epoch_micros();
        f64 delta_time = (f64)(new_time - last_time);
        last_time = new_time;

        // Cap frame time to prevent spiral of death
        if (delta_time > FRAME_CAP) {
            TLWARN("Frame time %.2f ms exceeded, capping to %.2f ms", delta_time / 1000.0, FRAME_CAP / 1000.0);
            delta_time = FRAME_CAP;
        }

        fps_timer += delta_time;
        accumulator += delta_time;
        while (accumulator >= STEP) {
            // update(STEP);
            update_count++;
            accumulator -= STEP;
        }

        const f64 alpha = accumulator / STEP;
        // render(alpha);
        (void)alpha;  // Suppress unused variable warning until render is implemented

        frame_count++;
        if (fps_timer >= ONE_SECOND_MICROS) {
            TLINFO("FPS: %llu | UPS: %llu", frame_count, update_count);

            frame_count = update_count = 0;
            fps_timer -= ONE_SECOND_MICROS;
        }
    }

    TL_PROFILER_POP_WITH(true)
}

b8 tl_application_terminate(void) {
    TL_PROFILER_PUSH
    TLTRACE("tl_application_terminate()")
    TL_PROFILER_POP_WITH(true)
}